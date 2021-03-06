const express = require('express');
const readline = require('readline');
var bodyParser = require("body-parser");
const os = require('os');
const fs = require('fs-extra');
const multer = require('multer');
const FormData = require('form-data');
const pathmod = require('path');

const server = express()
const rl = readline.createInterface({
  input: process.stdin,
  output: process.stdout
});

const hostname = os.hostname();
const DEFAULT_DIRECTORY = __dirname + "/store/";

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
server.use('/list', bodyParser.json());
server.use('/want', bodyParser.json());

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

server.post("/list", (req, res) => {
	let path = DEFAULT_DIRECTORY + req.body.path;
	console.log(`Finding files at path: ${path}\nIn path: ${__dirname}`);
	fs.readdir(path, (err, files) => {
		if(err){
			console.log("Incorrect path");
			res.send(JSON.stringify({error: true}));
		}else{console.log(`Found files: ${files}`);
		let fileNames = "";
		files.forEach(file => {fileNames += file + "\t"});
		res.send(JSON.stringify({error: false, message: fileNames}));
		}		
	})
});

server.post("/take", (req, res) => {
	let upload = multer({dest: DEFAULT_DIRECTORY}).single('file');

	upload(req, res, (err) => {
		if(err) {
			return res.send(JSON.stringify({error: true, message: err.message}));
		}
		console.log(`Saving file to path: /${req.body.path}`);
		fs.copy(req.file.path, DEFAULT_DIRECTORY + req.body.path, err => {
			if(err){
				return res.send(JSON.stringify({error: true, message: err.message}));
			}
			fs.unlinkSync(req.file.path);
			res.send(JSON.stringify({error: false, message: "File received successfully"}));
		})
	})
});

server.post("/want", (req, res) => {
	console.log(`Client asked for: ${req.body.path}`);
	let path = DEFAULT_DIRECTORY + req.body.path;
	const form = new FormData();
	if(!fs.existsSync(path)){
		form.append('error', 'true');
		form.append('message', 'requested file does not exist');
		res.writeHead(404, form.getHeaders());
		form.pipe(res);
	}else{
		let file = fs.createReadStream(path, {encoding: null});
		let name = pathmod.basename(path);
		console.log(`Found file name: ${name}`);
		form.append('name', name);
		form.append('file', file);
		res.writeHead(200, form.getHeaders());
		form.pipe(res);
	}
});

console.log(`Starting server on ${hostname} on port ${port}`);
server.listen(port);