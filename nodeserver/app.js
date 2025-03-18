const sqlite3 = require('sqlite3').verbose();
var WebSocketServer = require('ws').Server
  , wss = new WebSocketServer({ port: 5001 });
const fs = require('fs');

const db_name = "database/users.db";
const max_username_len = 20;
const event_codes = {
    "LOGIN_FAILURE": 3000,
}

var clicks = 0;


wss.on("connection", async function connection(ws) {
    console.log('New WebSocket connection established. Requesting login...');
    ws.send(JSON.stringify({"login_request": true}));

    // WebSocket handling logic
    ws.on("message", async function message(data, isBinary) {
        const message = isBinary ? data : data.toString();
        // Continue as before.
        // Handle WebSocket messages
        
        console.log("Message received", message)
        var response = await handle_message(message);
        var to_send = JSON.stringify(response);
        if (response["close"]) {
            ws.close(event_codes[response["code"]], to_send);
        } else {
            if (response["broadcast"]) {
                wss.clients.forEach(function each(client) {
                    if (client.readyState === WebSocket.OPEN) {
                        client.send(to_send);
                    }
                });
            } else {
                ws.send(to_send);
            }
        }
    });
    
    ws.on("close", async function close() {
        console.log("Connection closed");
    });
});

async function handle_message(data) {
    var message = JSON.parse(data);
    var request = message['request'];
    var response;
    if (request == 'login') {
        response = await login(message);
        console.log("login complete?")
        if (response["close"]) {
            response["code"] = event_codes["LOGIN_FAILURE"];
        }
    }
    if (request == 'click') {
        clicks += 1;
        response = {"message": `${clicks} clicks`, "broadcast": true};
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
    console.log("Created table or database?");
}

// assumes db is open
async function db_get_one(database, sql, params) {
    return new Promise((resolve, reject) => {
        database.get(sql, params, (err, row) => {
            if (err) {
                console.log("Query failed: ", sql);
                return resolve(undefined);
            } else {
                console.log("found row...", row, " from ", sql);
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
                response["login_success"] = true;
                response["username"] = username;
                response["message"] = `Logged in as ${username}.`
            } else {
                if (await db_get_one(db, 'SELECT * FROM users WHERE username = ?', [username])) {
                    response["close"] = true;
                    response["message"] = `Login failed.\nCould not update username: \"${username}\" is already taken.`
                } else {
                    db_run(db, 'UPDATE users SET username = ? WHERE account = ? AND watch = ?', [username, account, watch]);
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