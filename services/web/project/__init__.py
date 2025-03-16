from flask import Flask, jsonify, request
import sqlite3

db_path = "/var/etc/database/data.db"
max_username_len = 20

app = Flask(__name__)


@app.route("/")
def welcome():
    return jsonify(message="Welcome to Pebbleland!")

@app.route("/dump")
def dump_db():
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()
    cursor.execute("SELECT * FROM users")
    result = "<br>".join(" ".join(x) for x in cursor.fetchall())
    conn.commit()
    conn.close()
    return result

def create_user_database():
    conn = sqlite3.connect(db_path)
    c = conn.cursor()
    c.execute('''CREATE TABLE IF NOT EXISTS users (account TEXT, watch TEXT, username TEXT)''')
    conn.commit()
    conn.close()

def attempt_login(account, watch, username):
    if not username:
        return False, "Login failed.\nPlease set your username."
    elif len(username) > max_username_len:
        return False, f"Login failed.\nUsername must be less than {len(username)} characters."
    if not account:
        return False, "Login failed.\nAccount token not provided."
    if not watch:
        return False, "Login failed.\nWatch token not provided."
    
    create_user_database()

    conn = sqlite3.connect(db_path)
    c = conn.cursor()
    c.execute('SELECT username FROM users WHERE account = ? AND watch = ?', (account, watch,))
    result = c.fetchone()
    message = ""
    logged_in = True
    if result:
        db_username = result[0]
        if username == db_username:
            message = f'Logged in as {username}.'
        else:
            c.execute('SELECT * FROM users WHERE username = ?', (username, ))
            if c.fetchall():
                logged_in = False
                message = f"Login failed.\nCould not update username: \"{username}\" is already taken."
            else:
                c.execute('UPDATE users SET username = ? WHERE account = ? AND watch = ?', (username, account, watch))
                message = f'Login successful.\nUpdated username from {db_username} to {username}'
    else:
        c.execute('SELECT * FROM users WHERE username = ?', (username, ))
        if c.fetchall():
            logged_in = False
            message = f"Login failed.\nCould not create account: username \"{username}\" is already taken."
        else:
            c.execute('INSERT INTO users VALUES (?, ?, ?)', (account, watch, username))
            message = f'Login successful.\nCreated account for {username}'
    conn.commit()
    conn.close()
    return logged_in, message

@app.route("/login", methods=['POST', 'GET'])
def login():
    if request.method == 'POST':
        data = request.get_json()
        watch_token = data.get('watch_token')
        account_token = data.get('account_token')
        username = data.get('username')

        login_result, message = attempt_login(account_token, watch_token, username)
        response = {}
        response['message'] = message
        response['login_successful'] = login_result
        response['username'] = username
        return jsonify(response)
        # return jsonify({"user": username, "account": account_token, "watch": watch_token})
    else:
        return jsonify(message="Get requests not supported")
