var base_url = "http://10.0.0.47:5001";
// var base_url = "localhost:5001";

// Import the Clay package
var Clay = require("pebble-clay");
// Load our Clay configuration file
var clayConfig = require("./config");
// Initialize Clay
var clay = new Clay(clayConfig);

var socket;
var set_username;

var s_username;
var s_password;
var s_email;
var s_token = null;
var login_awaiting_username = false;

const ACC_MOD = "";
const WAT_MOD = ACC_MOD;

const ERROR_CODES = Object.freeze({
    // auth
    no_token: 4000,
    invalid_token: 4001,
    invalid_credentials: 4002,

    // lookup
    unknown_user: 4003,
    user_exists: 4004,
    username_taken: 4005,

    // other
    generic: 4099
});

const PATHS = Object.freeze({
    login: base_url + "/login",
    register: base_url + "/register",
    my_info: base_url + "/my_info",
    update_username: base_url + "/update_username",
    update_email: base_url + "/update_email"
});

// login:
// http POST localhost:5001/login watch_id=w account_id=b password=test
// -> success: 
//    initiate ws connection with {"request": "auth", "token": token}
// -> unknown user:
//    try register
//      -> success
//         login
//      -> fail
//         error for given reason
// -> invalid credentials:
//    "login failed: invalid credentials"
// 
               
// changes:
// update no longer sends player_location
// password
// recovery email
// if user_connected is me, update my info


// request data from url
var xhrGetRequest = function (url, token, callback) {
	var xhr = new XMLHttpRequest();
	xhr.onload = function () {
		callback(this.responseText);
	};
	xhr.open("GET", url);
    if (token != null) {
        xhr.setRequestHeader("Authorization", "Bearer " + token);
    }
    console.log("get: " + url);
	xhr.send();
};

var xhrPostRequest = function (url, data, token, callback) {
	var xhr = new XMLHttpRequest();
	xhr.onload = function () {
		callback(this.responseText);
	};
	xhr.open("POST", url);
    xhr.setRequestHeader("Content-Type", "application/json");
    if (token != null) {
        xhr.setRequestHeader("Authorization", "Bearer " + token);
    }
    console.log("post: " + JSON.stringify(data));
    xhr.send(JSON.stringify(data));
}

function send_to_pebble(dictionary) {
    Pebble.sendAppMessage(dictionary,
        function(e) {
            console.log("Message sent to Pebble successfully!", JSON.stringify(dictionary));
        },
        function(e) {
            console.log("Error sending message to Pebble!", JSON.stringify(dictionary));
        }
    );
}

function send_to_server(dictionary) {
    console.log("Sending: ", JSON.stringify(dictionary));
    socket.send(JSON.stringify(dictionary));
}

function complete_login(username) {
    dictionary = {
        "LoginSuccessful": true,
        "Username": username,
    };
    console.log("Logged in as " + username);
    send_to_pebble(dictionary);
}

function login(username, password, email, try_register) {
    var login_info = {
        "account_id": Pebble.getAccountToken() + ACC_MOD,
        "watch_id": Pebble.getWatchToken() + WAT_MOD,
        "password": password
    };
    xhrPostRequest(PATHS.login, login_info, null, function(response) {
        login_response_handler(response, username, password, email, try_register);
    });
}

function login_response_handler(response, username, password, email, try_register) {
    console.log("Login response: " + response);
    var json_response = JSON.parse(response);
    if ("token" in json_response) {
        console.log("Got token! " + json_response.token);
        s_token = json_response.token;
        xhrGetRequest(PATHS.my_info, s_token, function(response) {
            console.log("My info: " + response);
        });
        var server_username = json_response.username;
        var server_email = json_response.email;
        // TODO: consider adding these in
        if (server_username != username) {
            // // Update username
            login_awaiting_username = true;
            var username_info = {"username": username};
            xhrPostRequest(PATHS.update_username, username_info, s_token, function(response) {
                username_update_handler(response);
            });
        }
        if (server_email != email) {
            // Update email
            var email_info = {"email": email};
            xhrPostRequest(PATHS.update_email, email_info, function(response) {
                email_update_handler(response);
            });
        }
        if (!login_awaiting_username) {
            complete_login(username);
        }
    } else {
        console.log("Got error: " + json_response.error);
        switch (json_response.error) {
            case ERROR_CODES.unknown_user:
                console.log("User not found");
                if (try_register) {
                    console.log("Registering...");
                    register(username, password, email);
                } else {
                    send_to_pebble({"Message": "Login failed. Reason: user does not exist"});
                }
                break;
            case ERROR_CODES.invalid_credentials:
                console.log("Invalid credentials");
                send_to_pebble({"Message": "Login failed. Reason: Invalid credentials"});
                break;
            case ERROR_CODES.generic:
                console.log("Error: " + json_response.message);
                send_to_pebble({"Message": "Error: " + json_response.message});
                break;
            default:
                console.log("Unhandled error");
                send_to_pebble({"Message": "Unhandled error: " + json_response.error + ": " + json_response.message});
                break;
        }
    }
}

function register(username, password, email) {
    var registration_info = {
        "account_id": Pebble.getAccountToken() + ACC_MOD,
        "watch_id": Pebble.getWatchToken() + WAT_MOD,
        "username": username,
        "password": password,
        "email": email,
    };
    xhrPostRequest(PATHS.register, registration_info, null, function(response) {
        registration_response_handler(response, username, password, email);
    });
}

function registration_response_handler(response, username, password, email) {
    console.log("Registration response: ", response);
    var json_response = JSON.parse(response);
    if ("registration_success" in json_response) {
        console.log("Registration success, logging in");
        login(username, password, email, false);
    } else {
        console.log("Got error: " + json_response.error);
        switch (json_response.error) {
            case ERROR_CODES.user_exists:
                console.log("User already exists");
                send_to_pebble({"Message": "Registration failed. Reason: user already exists"});
                break;
            case ERROR_CODES.username_taken:
                console.log("Username already taken");
                send_to_pebble({"Message": "Registration failed. Reason: username \"" + username + "\" already taken"});
                break;
            case ERROR_CODES.generic:
                console.log("Error: " + json_response.message);
                send_to_pebble({"Message": "Error: " + json_response.message});
                break;
            default:
                console.log("Unhandled error");
                break;
        }
    }
}

function username_update_handler(response) {
    console.log("Username update response: ", response);
    var json_response = JSON.parse(response);
    if ("username_updated" in json_response) {
        if (json_response.username_updated) {
            console.log("Username updated");
            if (login_awaiting_username) {
                complete_login(json_response.username);
            }
            // send_to_pebble({"Message": "Username update failed. Reason: username \"" + json_response.new_username + "\" already taken. Using previous username: \"" + json_response.old_username + "\""});
        } else {
            if (json_response.old_username != json_response.new_username) {
                send_to_pebble({"Message": "Username update failed. Reason: username \"" + json_response.new_username + "\" already taken. Using previous username: \"" + json_response.old_username + "\""});
            }
        }
    } else {
        console.log("Got error: " + json_response.error);
        switch (json_response.error) {
            case ERROR_CODES.generic:
                console.log("Error: " + json_response.message);
                send_to_pebble({"Message": "Error: " + json_response.message});
                break;
            default:
                console.log("Unhandled error");
                break;
        }
    }
}

function broadcast_location(x, y, dir) {
    var location_info = {
        "request": "location",
        "x": x,
        "y": y,
        "dir": dir,
    };
    send_to_server(location_info);
}

function broadcast_update(dict) {
    var connect_info = {
        "request": "update",
        "x": dict["X"],
        "y": dict["Y"],
        "dir": dict["Dir"],
        "hair_style": dict["HairStyle"],
        "shirt_style": dict["ShirtStyle"],
        "pants_style": dict["PantsStyle"],
        "hair_color": dict["HairColor"],
        "shirt_color": dict["ShirtColor"],
        "pants_color": dict["PantsColor"],
        "shoes_color": dict["ShoesColor"],
    };
    send_to_server(connect_info);
}

function broadcast_connect(dict) {
    var connect_info = {
        "request": "user_connected",
        "x": dict["X"],
        "y": dict["Y"],
        "dir": dict["Dir"],
        "hair_style": dict["HairStyle"],
        "shirt_style": dict["ShirtStyle"],
        "pants_style": dict["PantsStyle"],
        "hair_color": dict["HairColor"],
        "shirt_color": dict["ShirtColor"],
        "pants_color": dict["PantsColor"],
        "shoes_color": dict["ShoesColor"],
    };
    send_to_server(connect_info);
}

function poll() {
    var connect_info = {
        "request": "poll_users",
    };
    send_to_server(connect_info);
}

function handle_event(event) {
    var dictionary;
    if (event["reason"] == "login_request") {
        login(set_username);
    } else if (event["reason"] == "login_success") {
        dictionary = {
            // "Message": event["message"],
            "LoginSuccessful": true,
            "Username": event["username"],
            "Source": event["source"]
        };
        console.log(event["message"]);
        send_to_pebble(dictionary);
    } else if (event["reason"] == "user_connected") {
        dictionary = {
            "UserConnected": true,
            "Username": event["source"],
            "X": event["x"],
            "Y": event["y"],
            "Dir": event["dir"],
            "HairStyle": event["hair_style"],
            "ShirtStyle": event["shirt_style"],
            "PantsStyle": event["pants_style"],
            "HairColor": event["hair_color"],
            "ShirtColor": event["shirt_color"],
            "PantsColor": event["pants_color"],
            "ShoesColor": event["shoes_color"],
        };
        send_to_pebble(dictionary);
    } else if (event["reason"] == "user_disconnected") {
        dictionary = {
            "UserDisconnected": true,
            "Username": event["source"],
        };
        send_to_pebble(dictionary);
    } else if (event["reason"] == "polled_users") {
        var users = event["users"];
        console.log("Polling users: ", JSON.stringify(users));
        for (var user in users) {
            console.log("User:", user);
            console.log("User info", JSON.stringify(users[user]));
            dictionary = {
                "UserConnected": true,
                "Username": user,
                "X": users[user]["x"],
                "Y": users[user]["y"],
                "Dir": users[user]["dir"],
                "HairStyle": users[user]["hair_style"],
                "ShirtStyle": users[user]["shirt_style"],
                "PantsStyle": users[user]["pants_style"],
                "HairColor": users[user]["hair_color"],
                "ShirtColor": users[user]["shirt_color"],
                "PantsColor": users[user]["pants_color"],
                "ShoesColor": users[user]["shoes_color"],
            };
            send_to_pebble(dictionary);
        };
    } else if (event["reason"] == "location") {
        dictionary = {
            "Location": true,
            "Username": event["source"],
            "X": event["x"],
            "Y": event["y"],
            "Dir": event["dir"]
        };
        send_to_pebble(dictionary);
    } else if (event["reason"] == "update") {
        dictionary = {
            "Update": true,
            "Username": event["source"],
            "X": event["x"],
            "Y": event["y"],
            "Dir": event["dir"],
            "HairStyle": event["hair_style"],
            "ShirtStyle": event["shirt_style"],
            "PantsStyle": event["pants_style"],
            "HairColor": event["hair_color"],
            "ShirtColor": event["shirt_color"],
            "PantsColor": event["pants_color"],
            "ShoesColor": event["shoes_color"],
        };
        send_to_pebble(dictionary);
    } else {
        dictionary = {
            "Message": event["message"]
        };
        send_to_pebble(dictionary);
    }
}

function connect_websocket() {
    var constructed_webserver_url = "ws://" + base_url;
    console.log("trying ws: " + constructed_webserver_url);
    
    socket = new WebSocket(constructed_webserver_url);
    
    socket.onopen = function(event) {
        console.log("[open] Connection established");
    };
    
    socket.onmessage = function(event, isBinary) {
        var text = event.data instanceof ArrayBuffer ? event.data.toString() : event.data;
        console.log("received server message: ", text.toString());
        var json = JSON.parse(text);

        handle_event(json);
    };
    
    socket.onclose = function(event) {
        if (event.wasClean) {
            console.log("[close] Connection closed cleanly, code=" + event_codes[event.code] + " reason=" + event.reason);
            if (event_codes[event.code] == "LOGIN_FAILURE") {
                var dictionary = {
                    "Message": event.reason,
                    "LoginSuccessful": false
                };
                send_to_pebble(dictionary);
            }
        } else {
            // e.g. server process killed or network down
            // event.code is usually 1006 in this case
            console.log("[close] Connection died");
        }
    };
    
    socket.onerror = function(error) {
        console.log("[error]");
    };
}

function disconnect_websocket() {
    if (socket) {
        send_to_server({"request": "close"});
        console.log("Closing websocket...");
        socket.close(); // This causes error on server, don"t know why, don"t care B) (aka just closing socket on error on server side)
    } else {
        console.log("Not connected to websocket!");
    }
}

// send nothing to pebble, which will prompt weather request
function pokePebble() {
	var dictionary = {};
	Pebble.sendAppMessage(dictionary,
		function(e) {
			console.log("Pebble poked successfully!");
		},
		function(e) {
			console.log("Error poking Pebble!");
		}
	);
}

// Listen for when the watchface is opened
Pebble.addEventListener("ready",
	function(e) {
		console.log("PebbleKit JS ready!");
	}
);

// Listen for when an AppMessage is received
Pebble.addEventListener("appmessage",
	function(e) {
        var dict = e.payload;
		console.log("AppMessage received!", JSON.stringify(dict));

        // TODO: remove this once implemented on watch
        if (dict["RequestLogin"]) {
            login(
                dict["Username"],
                dict["Password"],
                dict["Email"],
                true
            );
        }
        if (dict["Connect"]) {
            set_username = dict["Username"];
            connect_websocket();
        }
        if (dict["Disconnect"]) {
            disconnect_websocket();
        }
        if (dict["BroadcastConnect"]) {
            broadcast_connect(dict);
        }
        if (dict["Location"]) {
            broadcast_location(dict["X"], dict["Y"], dict["Dir"]);
        }
        if (dict["Update"]) {
            broadcast_update(dict);
        }
        if (dict["Poll"]) {
            poll();
        }
        // {
        //     console.log("Unsupported request:", JSON.stringify(dict));
        // }// Poll users
        // 
	}
);

