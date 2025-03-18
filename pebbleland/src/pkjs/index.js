var base_url = "10.0.0.47:5001";

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
            console.log('Message sent to Pebble successfully!');
        },
        function(e) {
            console.log('Error sending message to Pebble!');
        }
    );
}

function send_to_server(dictionary) {
    console.log("Sending: ", JSON.stringify(dictionary));
    socket.send(JSON.stringify(dictionary));
}

function login(username) {
    var login_info = {
        'watch_token': Pebble.getAccountToken(),
        'account_token': Pebble.getWatchToken(),
        'username': username,
        // 'username': 'tester',
        'request': 'login'
    };

    send_to_server(login_info);
}

function handle_event(event) {
    var dictionary;
    if (event["login_request"]) {
        login(set_username);
    } else if (event["login_success"]) {
        dictionary = {
            'Message': event["message"],
            'LoginSuccessful': true,
            'Username': event["username"]
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
        console.log("received message: ", text.toString());
        var json = JSON.parse(text);

        console.log('Received websocket message: ' + JSON.stringify(json));

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
		console.log('AppMessage received!');
		var dict = e.payload;

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
	}
);

