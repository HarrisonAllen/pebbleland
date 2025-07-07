const sqlite3 = require('sqlite3').verbose();
const ws_import = require('ws');
const bcrypt = require('bcryptjs');
const express = require('express');
import { AppWebSocket } from "./app_web_socket";
import { app, set_up_express } from "./express";
import { set_up_routes } from "./routes";

// Initializers
set_up_express();
set_up_routes();

// Server initialization
const server = app.listen(5001);
// TODO: move websocket to its own file
// TODO: migrate db accesses to prisma
const wss:any = new ws_import.Server({ noServer: true });
server.on('upgrade', (request: any, socket: any, head: any) => {
    wss.handleUpgrade(request, socket, head, function (ws: WebSocket) {
        wss.emit('connection', ws, request);
    });
});
  
// TODO: server tick (4 ticks?)
// TODO: Only broadcast changes on server tick
// TODO: make server send queue

const db_name: string = "database/users.db";
const max_username_len: number = 20;
const event_codes: {[key: string] : number} = {
    "OK": 1000,
    "LOGIN_FAILURE": 3000,
    "MISC_ERROR": 3001
}

var users: {[key: string] : AppWebSocket} = {};

async function broadcast(to_send: string) {
    console.log('Broadcasting: ', to_send);
    wss.clients.forEach(function each(client: WebSocket) {
        if (client.readyState === WebSocket.OPEN) {
            client.send(to_send);
        }
    });
}

async function get_connected_users() {
    var user_dict: { [username: string] : { [item: string]: any;}; }= {}
    for (const [username, client] of Object.entries(users)) {
    // users.forEach(function each(username: string) {
        // var client = users[username];
        console.log("Polling: ", username, " aka ", client.username);
        if (client.username !== undefined) 
            user_dict[client.username] = Object.assign(
                {},
                client.get_player_data(),
                client.get_player_location()
            );
    };
    return user_dict;
}

// TODO:
// Figure out this error: code: 'WS_ERR_INVALID_CLOSE_CODE',
//                              [Symbol(status-code)]: 1002

wss.on("connection", async function connection(ws: WebSocket) {
    console.log('New WebSocket connection established. Requesting login...');
    var aws = new AppWebSocket(ws);
    aws.ws.send(JSON.stringify({"reason": "login_request"}));

    // WebSocket handling logic
    // @ts-expect-error
    aws.ws.on("message", async function message(data: string|number, isBinary: boolean) {
        const message = isBinary ? data : data.toString();
        // Continue as before.
        // Handle WebSocket messages
        
        console.log("Message received", message)
        var response = await handle_message(message, aws);
        var to_send = JSON.stringify(response);
        if (response["broadcast"]) {
            broadcast(to_send);
        } else {
            console.log('Sending:', to_send);
            aws.ws.send(to_send);
        }
        if (response["close"]) {
            if (aws.ws.readyState === WebSocket.OPEN) { // maybe error is closing on already closed socket?
                aws.ws.close(event_codes[response["code"]], to_send);
            }
        }
    });
    
    // @ts-expect-error
    aws.ws.on("close", async function close() {
        if (aws.username !== undefined && users[aws.username] && users[aws.username].ws.readyState === WebSocket.CLOSED) {
            console.log("Connection for", aws.username, "closed");
            var response = {"reason": "user_disconnected", "source": aws.username, "broadcast": true};
            var to_send = JSON.stringify(response);
            broadcast(to_send);
            delete users[aws.username];
        } else {
            console.log("Disconnect received but websocket still active");
        }
    });

    // @ts-expect-error
    aws.ws.on("error", async function (event: any) {
        console.log("Websocket error for user", aws.username, ". Closing connection...");
        console.log("Provided error:", event);
        aws.ws.close(event_codes["MISC_ERROR"], "Websocket connection error");
    });
});

async function handle_message(data: any, aws: AppWebSocket) {
    var message: {[key: string] : any} = JSON.parse(data);
    var request: string = message['request'];
    var response: {[key: string] : any}= {};
    if (request == 'login') {
        response = await login(message);
        if (response["close"]) {
            response["code"] = event_codes["LOGIN_FAILURE"];
        }
        if (response["login_success"]) {
            aws.username = response["username"];
            if (aws.username !== undefined) {
                users[aws.username] = aws;
            }
        }
    }
    if (request == 'close') {
        response = {'close': true, 'code': event_codes["OK"]};
    }
    if (request == 'location') {
        aws.set_player_location(message);

        response = Object.assign(
            {},
            {
                "reason": "location",
                "source": aws.username,
                "broadcast": true
            },
            aws.get_player_location()
        )
    }
    if (request == 'update') {
        aws.set_player_data(message);
        response = Object.assign(
            {},
            {
                "reason": "update",
                "source": aws.username,
                "broadcast": true
            },
            aws.get_player_data(),
            aws.get_player_location()
        );
    }
    if (request == 'user_connected') {
        aws.set_player_data(message);
        aws.set_player_location(message);
        response = Object.assign(
            {},
            {
                "reason": "user_connected",
                "source": aws.username,
                "broadcast": true
            },
            aws.get_player_data(),
            aws.get_player_location()
        )
    }
    if (request == 'poll_users') {
        var user_dict = await get_connected_users();
        response = {"reason": "polled_users", "users": user_dict};
    }

    return response
}

async function run_db_command(command: string) {
    const db = new sqlite3.Database(db_name);

    db.exec(command, (err: any) => {
        if (err) {
            console.log("Failed to execute command ", err);
        }
    });

    db.close();
}

async function create_user_table() {
    run_db_command('CREATE TABLE IF NOT EXISTS users (account TEXT, watch TEXT, username TEXT)');
}

// assumes db is open
// @ts-expect-error
async function db_get_one(database: sqlite3.Database, sql: string, params: any[]): Promise<{[key: string] : any}|undefined> {
    return new Promise((resolve, reject) => {
        database.get(sql, params, (err: any, row: {[key: string] : any}) => {
            if (err) {
                console.log("Query failed: ", sql);
                return resolve(undefined);
            } else {
                return resolve(row);
            }
        });
    });
}

// @ts-expect-error
async function db_run(database: sqlite3.Database, sql: string, params: any[]) {
    database.run(sql, params, (err: any) => {
        if (err) {
            console.log("Run failed: ", sql);
        }
    });
}

async function login(message: {[key: string] : any}): Promise<{[key: string] : any}> {
    var watch: string = message['watch_token'];
    var account: string = message['account_token'];
    var username: string = message['username'];
    var response: {[key: string] : any} = {"login_success": false, "close": false};
    var login_failed = false;
    
    if (watch === undefined) {
        response["close"] = true;
        response["message"] = "Login failed.\nWatch token not provided.";
    }
    if (account === undefined) {
        response["close"] = true;
        response["message"] = "Login failed.\nAccount token not provided.";
    }
    if (username === undefined || username === "") {
        response["close"] = true;
        response["message"] = "Login failed.\nPlease set your username.";
    } else if (username.length > max_username_len) {
        response["close"] = true;
        response["message"] = `Login failed.\nUsername must be less than ${max_username_len} characters.`
    }
    
    if (!response["close"]) {        
        create_user_table();
        
        const db = new sqlite3.Database(db_name);
        
        var user_info: {[key: string] : any}|undefined = await db_get_one(db, 'SELECT username FROM users WHERE account = ? AND watch = ?', [account, watch]);
        if (user_info) {
            var db_username = user_info['username'];
            if (db_username === username) {
                response["reason"] = "login_success";
                response["login_success"] = true;
                response["username"] = username;
                response["message"] = `Logged in as ${username}.`
            } else {
                if (await db_get_one(db, 'SELECT * FROM users WHERE username = ?', [username])) {
                    response["close"] = true;
                    response["message"] = `Login failed.\nCould not update username: \"${username}\" is already taken.`
                } else {
                    db_run(db, 'UPDATE users SET username = ? WHERE account = ? AND watch = ?', [username, account, watch]);
                    response["reason"] = "login_success"
                    response["login_success"] = true;
                    response["username"] = username;
                    response["message"] = `Login successful.\nUpdated username from ${db_username} to ${username}`
                }
            }
        } else {
            if (await db_get_one(db, 'SELECT * FROM users WHERE username = ?', [username])) {
                response["close"] = true;
                response["message"] = `Login failed.\nCould not create account: \"${username}\" is already taken.`
            } else {
                db_run(db, 'INSERT INTO users VALUES (?, ?, ?)', [account, watch, username]);
                response["reason"] = "login_success"
                response["login_success"] = true;
                response["username"] = username;
                response["message"] = `Login successful.\nCreated account for ${username}`
            }
        }

        db.close();

        // response["close"] = login_failed;

        
    }
    
    return response;

}