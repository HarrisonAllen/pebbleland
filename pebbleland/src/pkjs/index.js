var url = "http://10.0.0.47:5001";

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
	xhrGetRequest(url,
		function(responseText) {
            console.log('received data: ' + responseText);

			// var dictionary = {
			// 	'TEMPERATURE': temperature,
			// 	'CONDITIONS': conditions
			// };
			// Pebble.sendAppMessage(dictionary,
			// 	function(e) {
			// 		console.log('Weather info sent to Pebble successfully!');
			// 	},
			// 	function(e) {
			// 		console.log('Error sending weather info to Pebble!');
			// 	}
			// );
		}
	);
}

function login() {
    var login_info = {
        'watch_token': Pebble.getAccountToken(),
        'account_token': Pebble.getWatchToken(),
        'username': 'LinkSky'
    };
    xhrPostRequest(url + "/login", login_info,
        function(responseText) {
            console.log('received data: ' + responseText);
            var json = JSON.parse(responseText);
            console.log('received message: ', json.message);
        }
    );
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

		// Get initial data
		login();
	}
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
	function(e) {
		console.log('AppMessage received!');
		var dict = e.payload;
	}
);
