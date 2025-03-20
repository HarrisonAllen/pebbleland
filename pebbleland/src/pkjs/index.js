var base_url = "10.0.0.21:5001";

// Import the Clay package
var Clay = require('pebble-clay');
// Load our Clay configuration file
var clayConfig = require('./config');
// Initialize Clay
var clay = new Clay(clayConfig);

var socket;
var set_username;

var event_codes = {
    1000: "OK",
    3000: "LOGIN_FAILURE",
};

// request data from url
var xhrGetRequest = function (url, callback) {
	var xhr = new XMLHttpRequest();
	xhr.onload = function () {
		callback(this.responseText);
	};
	xhr.open("GET", url);
	xhr.send();
};

var xhrPostRequest = function (url, data, callback) {
	var xhr = new XMLHttpRequest();
	xhr.onload = function () {
		callback(this.responseText);
	};
	xhr.open("POST", url);
    xhr.setRequestHeader("Content-Type", "application/json");
    console.log(JSON.stringify(data));
    xhr.send(JSON.stringify(data));
}

function mainPage() {
	xhrGetRequest('http://' + base_url,
		function(responseText) {
            console.log('received data: ' + responseText);
		}
	);
}

function send_to_pebble(dictionary) {
    Pebble.sendAppMessage(dictionary,
        function(e) {
            console.log('Message sent to Pebble successfully!', JSON.stringify(dictionary));
        },
        function(e) {
            console.log('Error sending message to Pebble!', JSON.stringify(dictionary));
        }
    );
}

function send_to_server(dictionary) {
    console.log("Sending: ", JSON.stringify(dictionary));
    socket.send(JSON.stringify(dictionary));
}

function login(username) {
    var login_info = {
        'request': 'login',
        'watch_token': Pebble.getAccountToken(),
        'account_token': Pebble.getWatchToken(),
        'username': username,
        // 'username': 'Emulator',
    };

    send_to_server(login_info);
}

function click(button) {
    var click_info = {
        'request': 'click',
        'button': button
    };
    send_to_server(click_info);
}

function broadcast_location(x, y) {
    var location_info = {
        'request': 'location',
        'x': x,
        'y': y
    };
    send_to_server(location_info);
}

function broadcast_connect(x, y) {
    var connect_info = {
        'request': 'user_connected',
        'x': x,
        'y': y,
    };
    send_to_server(connect_info);
}

function poll() {
    var connect_info = {
        'request': 'poll_users',
    };
    send_to_server(connect_info);
}

function handle_event(event) {
    var dictionary;
    if (event["reason"] == "login_request") {
        login(set_username);
    } else if (event["reason"] == "login_success") {
        dictionary = {
            // 'Message': event["message"],
            'LoginSuccessful': true,
            'Username': event["username"],
            'Clicks': event["clicks"],
            'Source': event["source"]
        };
        console.log(event["message"]);
        send_to_pebble(dictionary);
    } else if (event["reason"] == "click") {
        dictionary = {
            'Clicks': event["clicks"],
            'Source': event["source"]
        };
        send_to_pebble(dictionary);
    } else if (event["reason"] == "user_connected") {
        dictionary = {
            'UserConnected': true,
            'Username': event["source"],
            'PlayerX': event["x"],
            'PlayerY': event["y"]
        };
        send_to_pebble(dictionary);
    } else if (event["reason"] == "user_disconnected") {
        dictionary = {
            'UserDisconnected': true,
            'Username': event["source"],
        };
        send_to_pebble(dictionary);
    } else if (event["reason"] == "polled_users") {
        var users = event["users"];
        console.log("Polling users: ", JSON.stringify(users));
        for (var user in users) {
            console.log("User:", user);
            console.log("User info", JSON.stringify(users[user]));
            dictionary = {
                'UserConnected': true,
                'Username': user,
                'PlayerX': users[user]['x'],
                'PlayerY': users[user]['y']
            };
            send_to_pebble(dictionary);
        };
    } else if (event["reason"] == "location") {
        dictionary = {
            'Location': true,
            'Username': event["source"],
            'PlayerX': event["x"],
            'PlayerY': event["y"]
        };
        send_to_pebble(dictionary);
    } else {
        dictionary = {
            'Message': event["message"]
        };
        send_to_pebble(dictionary);
    }
}

function connect_websocket() {
    var constructed_webserver_url = 'ws://' + base_url;
    console.log('trying ws: ' + constructed_webserver_url);
    
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
            console.log('[close] Connection closed cleanly, code=' + event_codes[event.code] + ' reason=' + event.reason);
            if (event_codes[event.code] == "LOGIN_FAILURE") {
                var dictionary = {
                    'Message': event.reason,
                    'LoginSuccessful': false
                };
                send_to_pebble(dictionary);
            }
        } else {
            // e.g. server process killed or network down
            // event.code is usually 1006 in this case
            console.log('[close] Connection died');
        }
    };
    
    socket.onerror = function(error) {
        console.log('[error]');
    };
}

function disconnect_websocket() {
    if (socket) {
        console.log('Closing websocket...');
        send_to_server({"request": "close"});
        // socket.close(); // This breaks server when sending from watch... idk man
    } else {
        console.log('Not connected to websocket!');
    }
}

// send nothing to pebble, which will prompt weather request
function pokePebble() {
	var dictionary = {};
	Pebble.sendAppMessage(dictionary,
		function(e) {
			console.log('Pebble poked successfully!');
		},
		function(e) {
			console.log('Error poking Pebble!');
		}
	);
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready',
	function(e) {
		console.log('PebbleKit JS ready!');
	}
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
	function(e) {
        var dict = e.payload;
		console.log('AppMessage received!', JSON.stringify(dict));

        if (dict['RequestLogin']) {
            login(dict['Username']);
        }
        if (dict['Connect']) {
            set_username = dict['Username'];
            connect_websocket();
        }
        if (dict['Disconnect']) {
            disconnect_websocket();
        }
        if (dict['Click']) {
            click(dict['Button']);
        }
        if (dict['BroadcastConnect']) {
            broadcast_connect(dict['PlayerX'], dict['PlayerY']);
        }
        if (dict['Location']) {
            broadcast_location(dict['PlayerX'], dict['PlayerY']);
        }
        if (dict['Poll']) {
            poll();
        }
        // {
        //     console.log('Unsupported request:', JSON.stringify(dict));
        // }// Poll users
        // 
	}
);

