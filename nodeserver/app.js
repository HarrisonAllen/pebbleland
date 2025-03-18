const sqlite3 = require('sqlite3').verbose();
var WebSocketServer = require('ws').Server
  , wss = new WebSocketServer({ port: 5001 });
const fs = require('fs');

const db_name = "database/users.db";
const max_username_len = 20;
const event_codes = {
    "OK": 1000,
    "LOGIN_FAILURE": 3000,
}

var clicks = 0;
var last_click = "No one!";
var users = {};



wss.on("connection", async function connection(ws) {
    console.log('New WebSocket connection established. Requesting login...');
    ws.send(JSON.stringify({"reason": "login_request"}));

    // WebSocket handling logic
    ws.on("message", async function message(data, isBinary) {
        const message = isBinary ? data : data.toString();
        // Continue as before.
        // Handle WebSocket messages
        
        console.log("Message received", message)
        var response = await handle_message(message, ws);
        var to_send = JSON.stringify(response);
        if (response["close"]) {
            ws.close(event_codes[response["code"]], to_send);
        } else {
            if (response["broadcast"]) {
                console.log('Broadcasting: ', to_send);
                wss.clients.forEach(function each(client) {
                    if (client.readyState === WebSocket.OPEN) {
                        client.send(to_send);
                    }
                });
            } else {
                console.log('Sending: ', to_send);
                ws.send(to_send);
            }
        }
    });
    
    ws.on("close", async function close() {
        console.log("Connection closed");
        if (users[ws.username]) {
            delete users[ws.username];
        }
    });
});

async function handle_message(data, ws) {
    var message = JSON.parse(data);
    var request = message['request'];
    var response;
    if (request == 'login') {
        response = await login(message);
        if (response["close"]) {
            response["code"] = event_codes["LOGIN_FAILURE"];
        }
        if (response["login_success"]) {
            ws.username = response["username"];
            users[ws.username] = ws;
        }
        response["clicks"] = clicks;
        response["source"] = last_click;
    }
    if (request == 'close') {
        response = {'close': true, 'code': event_codes["OK"]};
    }
    if (request == 'click') {
        if (message['button'] == 'up') {
            clicks += 1;
        } else if (message['button'] == 'down') {
            clicks -= 1;
        } else {
            clicks = 0;
        }
        last_click = ws.username;
        response = {"reason": "click", "clicks": clicks, "source": last_click, "broadcast": true};
    }

    return response
}

async function run_db_command(command) {
    const db = new sqlite3.Database(db_name);

    db.exec(command, (err) => {
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
async function db_get_one(database, sql, params) {
    return new Promise((resolve, reject) => {
        database.get(sql, params, (err, row) => {
            if (err) {
                console.log("Query failed: ", sql);
                return resolve(undefined);
            } else {
                return resolve(row);
            }
        });
    });
}

async function db_run(database, sql, params) {
    database.run(sql, params, (err) => {
        if (err) {
            console.log("Run failed: ", sql);
        }
    });
}

async function login(message) {
    
    var watch = message['watch_token'];
    var account = message['account_token'];
    var username = message['username'];
    var response = {"login_success": false, "close": false};
    var login_failed = false;
    
    if (watch === undefined) {
        response["close"] = true;
        response["message"] = "Login failed.\nWatch token not provided.";
    }
    if (account === undefined) {
        response["close"] = true;
        response["message"] = "Login failed.\nAccount token not provided.";
    }
    if (username === undefined) {
        response["close"] = true;
        response["message"] = "Login failed.\nPlease set your username.";
    } else if (username.length > max_username_len) {
        response["close"] = true;
        response["message"] = `Login failed.\nUsername must be less than ${max_username_len} characters.`
    }
    
    if (!response["close"]) {        
        create_user_table();
        
        const db = new sqlite3.Database(db_name);
        
        var user_info = await db_get_one(db, 'SELECT username FROM users WHERE account = ? AND watch = ?', [account, watch]);
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

        response["close"] = login_failed;

        return response;
    }


}