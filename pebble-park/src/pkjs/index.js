// var base_url = "http://10.0.0.47:5001";
// var base_url = "192.0.0.2:5001";
var base_url = "localhost:5001";
var http_url = "http://" + base_url;
var ws_url = "ws://" + base_url;
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
var my_info_dict;

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
    login: http_url + "/login",
    register: http_url + "/register",
    my_info: http_url + "/my_info",
    update_username: http_url + "/update_username",
    update_email: http_url + "/update_email"
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

function complete_login() {
    // TODO: figure out why username change doesn't complete login, myinfo not requested?
    if (s_username != null && my_info_dict != null) {
        my_info_dict["LoginSuccessful"] = true;
        my_info_dict["Username"] = s_username;
        console.log("Logged in as " + s_username);
        console.log(JSON.stringify(my_info_dict));
        send_to_pebble(my_info_dict);
    } else {
        console.log("Not ready yet - User " + s_username + " info " + my_info_dict);
    }
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
            response = JSON.parse(response);
            my_info_dict = {
                "MyInfo": true,
                "HairStyle": response["user"]["playerInfo"]["hairStyle"],
                "ShirtStyle": response["user"]["playerInfo"]["shirtStyle"],
                "PantsStyle": response["user"]["playerInfo"]["pantsStyle"],
                "HairColor": response["user"]["playerInfo"]["hairColor"],
                "ShirtColor": response["user"]["playerInfo"]["shirtColor"],
                "PantsColor": response["user"]["playerInfo"]["pantsColor"],
                "ShoesColor": response["user"]["playerInfo"]["shoesColor"],
            };
            complete_login();
        });

        var server_username = json_response.username;
        var server_email = json_response.email;
        if (server_username != username) {
            // Update username
            login_awaiting_username = true;
            var username_info = {"username": username};
            xhrPostRequest(PATHS.update_username, username_info, s_token, function(response) {
                username_update_handler(response);
            });
        } else {
            s_username = username;
        }
        if (server_email != email) {
            // Update email
            var email_info = {"email": email};
            xhrPostRequest(PATHS.update_email, email_info, function(response) {
                email_update_handler(response);
            });
        }
        if (!login_awaiting_username) {
            complete_login();
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
                s_username = json_response.username;
                complete_login();
            }
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

function handle_websocket_message(message_json) {
    var send_dictionary = {};
    if (message_json.reason == "authentication") {
        if (message_json.authenticated) {
            send_dictionary = {
                "ConnectSuccessful": true
            };
        } else {
            send_dictionary = {
                "ConnectSuccessful": false,
                "Message": "Websocket authentication failed!"
            };
        }
        send_to_pebble(send_dictionary);
    } else if (message_json["reason"] == "user_connected") {
        dictionary = {
            "UserConnected": true,
            "Username": message_json["source"],
            "X": message_json["x"],
            "Y": message_json["y"],
            "Dir": message_json["dir"],
            "HairStyle": message_json["hair_style"],
            "ShirtStyle": message_json["shirt_style"],
            "PantsStyle": message_json["pants_style"],
            "HairColor": message_json["hair_color"],
            "ShirtColor": message_json["shirt_color"],
            "PantsColor": message_json["pants_color"],
            "ShoesColor": message_json["shoes_color"],
        };
        send_to_pebble(dictionary);
    } else if (message_json["reason"] == "user_disconnected") {
        dictionary = {
            "UserDisconnected": true,
            "Username": message_json["source"],
        };
        send_to_pebble(dictionary);
    } else if (message_json["reason"] == "polled_users") {
        var users = message_json["users"];
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
    } else if (message_json["reason"] == "location") {
        dictionary = {
            "Location": true,
            "Username": message_json["source"],
            "X": message_json["x"],
            "Y": message_json["y"],
            "Dir": message_json["dir"]
        };
        send_to_pebble(dictionary);
    } else if (message_json["reason"] == "update") {
        dictionary = {
            "Update": true,
            "Username": message_json["source"],
            "HairStyle": message_json["hair_style"],
            "ShirtStyle": message_json["shirt_style"],
            "PantsStyle": message_json["pants_style"],
            "HairColor": message_json["hair_color"],
            "ShirtColor": message_json["shirt_color"],
            "PantsColor": message_json["pants_color"],
            "ShoesColor": message_json["shoes_color"],
        };
        send_to_pebble(dictionary);
    } else {
        dictionary = {
            "Message": message_json["message"]
        };
        send_to_pebble(dictionary);
    }
}

function connect_websocket() {
    console.log("trying ws: " + ws_url);
    
    socket = new WebSocket(ws_url);
    
    socket.onopen = function(event) {
        console.log("[open] Connection established");
        console.log("Authenticating websocket...");
        send_to_server({"request": "auth", "token": s_token})
    };
    
    socket.onmessage = function(event, isBinary) {
        var text = event.data instanceof ArrayBuffer ? event.data.toString() : event.data;
        console.log("received server message: ", text.toString());
        var json = JSON.parse(text);

        handle_websocket_message(json);
    };
    
    socket.onclose = function(event) {
        console.log("Close: " + JSON.stringify(event));
        var dictionary = {
            "Message": "Disconnected from server"
        };
        send_to_pebble(dictionary);
        if (event.wasClean) {
            if (event.code == ERROR_CODES.invalid_token) {
                console.log("Websocket authentication failure");
            }
            console.log("[close] Connection closed cleanly");
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
        socket.close(); // This causes error on server, don't know why, don't care B) (aka just closing socket on error on server side)
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

        if (dict["RequestLogin"]) {
            login(
                dict["Username"],
                dict["Password"],
                dict["Email"],
                true
            );
        }
        if (dict["Connect"]) {
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
	}
);

