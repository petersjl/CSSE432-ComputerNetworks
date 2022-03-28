const express = require('express');
const readline = require('readline');
var bodyParser = require("body-parser");

const server = express()
const rl = readline.createInterface({
  input: process.stdin,
  output: process.stdout
});

let port;
if (process.argv[2] == null) {
	rl.question("Enter a port number (8080): ", (p) => {
		if (p == null || p == ""){
			port = 8080;
		}else{
			port = parseInt(p, 10);
		}
	})
}else {
	port = parseInt(process.argv[2], 10);
}

server.use('/', bodyParser.json());

server.post('/', (req, res) => {
	console.log(`Got message: ${req.body.message}`);
	let m = req.body.message.toUpperCase();
	console.log(`New message: ${m}`);
	res.send(JSON.stringify({message: m}));
});
console.log(`Starting server on ${server.name} on port ${port}`);
server.listen(port);