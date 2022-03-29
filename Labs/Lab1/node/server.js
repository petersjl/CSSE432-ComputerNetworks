const express = require('express');
const readline = require('readline');
var bodyParser = require("body-parser");
const os = require('os');

const server = express()
const rl = readline.createInterface({
  input: process.stdin,
  output: process.stdout
});

const hostname = os.hostname();

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

let dict = {};

server.use('/', bodyParser.json());

server.post('/', (req, res) => {
	console.log(`${req.hostname} says: ${req.body.message}`);
	let m = req.body.message.toUpperCase();
	if(dict[req.hostname] == null){
		dict[req.hostname] = 0;
	}
	dict[req.hostname] += 1;
	console.log(`Replying to ${req.hostname} message ${dict[req.hostname]} with message in upper case`);
	res.send(JSON.stringify({message: dict[req.hostname] + " " + m}));
});
console.log(`Starting server on ${hostname} on port ${port}`);
server.listen(port);