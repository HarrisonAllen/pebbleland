import { AppWebSocket } from "./app_web_socket";
import { authenticate_ws_token } from "./authentication";
import { ERROR_CODES } from "./error_handling";
import { prisma } from "./prisma";

const ws_import = require('ws');
const wss:any = new ws_import.Server({ noServer: true });
const users: {[key: string] : AppWebSocket} = {};

export function set_up_websockets(server:any) {
    server.on('upgrade', (request: any, socket: any, head: any) => {
        wss.handleUpgrade(request, socket, head, function (ws: WebSocket) {
            wss.emit('connection', ws, request);
        });
    });
    wss.on("connection", handle_connection);
}

async function broadcast(to_send: {[key: string] : any}) {
    console.log('Broadcasting: ', to_send);
    wss.clients.forEach(function each(client: WebSocket) {
        if (client.readyState === WebSocket.OPEN) {
            client.send(JSON.stringify(to_send));
        }
    });
}

export async function handle_connection(ws: WebSocket) {
    console.log('New WebSocket connection established.');
    const aws = new AppWebSocket(ws);
    // @ts-expect-error
    aws.ws.on("message", async function message(data: string|number, isBinary: boolean) {
        const message = isBinary ? data : data.toString();
        console.log("Message received", message);
        await handle_message(message, aws);
    });

    // @ts-expect-error
    aws.ws.on("close", async function close() {
        console.log("Disconnected");
        if (aws.username !== undefined && users[aws.username] && users[aws.username].ws.readyState === WebSocket.CLOSED) {
            console.log("Connection for", aws.username, "closed");
            broadcast(
                {
                    "reason": "user_disconnected", 
                    "source": aws.username
                }
            );
            delete users[aws.username];
        } else {
            console.log("Disconnect received but websocket still active");
        }
    });

    // @ts-expect-error
    aws.ws.on("error", async function (event: any) {
        console.log("Provided error:", event);
        aws.ws.close(ERROR_CODES.generic, "Websocket connection error");
    });
}

export async function handle_message(data: any, aws: AppWebSocket) {
    const message: {[key: string] : any} = JSON.parse(data);
    const request: string = message["request"];

    // Putting data requests
    if (request == "auth") {
        if (await authenticate(message, aws)) {
            await aws.load_player_data();
            aws.ws.send(
                JSON.stringify({
                    "reason": "authentication",
                    "authenticated": true
                })
            );
            users[String(aws.username)] = aws;
            broadcast(
                Object.assign(
                    {},
                    {
                        "reason": "user_connected",
                        "source": aws.username,
                        "broadcast": true
                    },
                    aws.get_player_data(),
                    aws.get_player_location()
                )
            );
        } else {
            aws.ws.send(
                JSON.stringify({
                    "reason": "authentication",
                    "authenticated": false
                })
            );    
        }
    }

    if (!aws.authenticated) {
        console.log("Not authenticated!");
        aws.ws.close(ERROR_CODES.invalid_token, "Not authenticated!");
        return;
    }

    if (request == "close") {
        aws.ws.close();
    }

    if (request == "location") {
        aws.set_player_location(message);
        broadcast(
            Object.assign(
                {},
                {
                    "reason": "location",
                    "source": aws.username,
                    "broadcast": true
                },
                aws.get_player_location()
            )
        );
    }

    if (request == "update") {
        aws.set_player_data(message);
        broadcast(
            Object.assign(
                {},
                {
                    "reason": "update",
                    "source": aws.username,
                    "broadcast": true
                },
                aws.get_player_data()
            )
        );
    }

    if (request == "load_in") {
    }

    // Fetching data requests
    if (request == "poll_users") {
        var user_dict = await get_connected_users();
        aws.ws.send(
            JSON.stringify({
                "reason": "polled_users",
                "users": user_dict
            })
        );    
    }
}

async function get_connected_users() {
    const user_dict: { [username: string] : { [item: string]: any;}; }= {}
    for (const [username, client] of Object.entries(users)) {
    // users.forEach(function each(username: string) {
        // const client = users[username];
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


async function authenticate(message: {[key: string] : any}, aws: AppWebSocket) {
    const auth_result:{[key: string] : any} = await authenticate_ws_token(message["token"]);
    if (auth_result["authenticated"]) {
        console.log("Connected as " + JSON.stringify(auth_result["user_id"]));
        aws.authenticated = true;
        aws.watch_id = auth_result["user_id"]["watch_id"];
        aws.account_id = auth_result["user_id"]["account_id"];
        return true;
    } else {
        console.log("Token error (" + auth_result["error"] + "). Closing connection...");
        aws.ws.close(auth_result["error"], "Authentication failed");
        return false;
    }
}