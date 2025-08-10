const WebSocket = require('ws');
const readline = require('readline');

// Create readline interface for user input
const rl = readline.createInterface({
  input: process.stdin,
  output: process.stdout
});

// Connect to the WebSocket server
const ws = new WebSocket('ws://localhost:5001');

// Connection opened
ws.on('open', () => {
  console.log('Connected to the WebSocket server');
  promptForMessage();
});

// Listen for messages from the server
ws.on('message', (message) => {
  console.log(`Server: ${message}`);
});

// Handle errors
ws.on('error', (error) => {
  console.error('WebSocket error:', error);
});

// Handle connection close
ws.on('close', () => {
  console.log('Disconnected from the server');
  process.exit(0);
});

// Function to prompt user for messages
function promptForMessage() {
  rl.question('Enter a message (or "exit" to quit): ', (message) => {
    if (message.toLowerCase() === 'exit') {
      ws.close();
      rl.close();
      return;
    }
    ws.send(JSON.parse(message));
    promptForMessage();
  });
}