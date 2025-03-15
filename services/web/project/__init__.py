from flask import Flask, jsonify, request


app = Flask(__name__)


@app.route("/")
def welcome():
    return jsonify(message="Welcome to Pebbleland!")

@app.route("/login", methods=['POST', 'GET'])
def login():
    if request.method == 'POST':
        data = request.get_json()
        watch_token = data['watch_token']
        account_token = data['account_token']
        username = data['username']
        return jsonify(message=f"Hi {username}! Your account token is {account_token} and your watch token is {watch_token}")
        # return jsonify({"user": username, "account": account_token, "watch": watch_token})
    else:
        return jsonify(message="Get requests not supported")
