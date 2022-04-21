const http = require('http');
const readline = require('readline');
const fs = require('fs-extra');
const FormData = require('form-data');
const pathmod = require('path');
const formidable = require('formidable');

const rl = readline.createInterface({
	input: process.stdin,
	output: process.stdout
});

const DEFAULT_DIRECTORY = __dirname + "/store/";

if (process.argv[3] == null){
	console.log("Usage: node clinet.js host port.");
	process.exit();
}

const listOptions = {
	hostname: process.argv[2],
	port: process.argv[3],
	path: '/list',
	method: 'POST',
	headers: {
		'Content-Type': 'application/json'
	}
};

const takeOptions = {
	hostname: process.argv[2],
	port: process.argv[3],
	path: '/take',
	method: 'POST',
	headers: {
	}
};

const wantOptions = {
	hostname: process.argv[2],
	port: process.argv[3],
	path: '/want',
	method: 'POST',
	headers: {
		'Content-Type': 'application/json'
	}
};

console.log(`Talking to host: ${listOptions.hostname} on port ${listOptions.port}\nType ';;;' to exit\n`);

function askQuestion(){
	rl.question("> ", function (p) {
	if (p == ";;;"){
		console.log('Bye!');
		process.exit();
	}else{
		let parts = p.split(" ");
		let command = parts[0].toLowerCase();
		switch (command){
			case "exit":
			case "q":
			case "quit": {
				process.exit();
			}break;
			case "ls": 
			case "list": {
				list(parts[1]);
			}break;
			case "iwant" : {
				if(!parts[1]) {
					console.log("\tUsage: iWant <file path on server> (destination on client)\n");
					askQuestion();
				}
				want(parts[1], parts[2]);
			}break;
			case "utake" : {
				if(!parts[1]) {
					console.log("\tUsage: uTake <file path on client> (destination on server)\n");
					askQuestion();
				}
				take(parts[1], parts[2]);
			}break;
			default : {
				console.log("Available commands:\nls (path)\niWant <file path on server> (destination on client)\nuTake <file path on client> (destination on server)\nexit");
				askQuestion();
			}
		}
	}
	})
}

function list(path){
	const req = http.request(listOptions, res => {  
		res.on('data', d => {
			let data = JSON.parse(d);
			if(data.error){
				console.log("Error: path not found");
			}else{
				console.log(`${"/" + (path || "")}> ${data.message}`);
			}
			askQuestion();
		})
	})
		
	req.on('error', error => {
		console.error(error)
	})

	req.write(JSON.stringify({path: path || ""}));
	req.end();
}

function take(from, to){
	let path = DEFAULT_DIRECTORY + from;
	if(!fs.existsSync(path)) {console.log(`\tError: file does not exist ${from}`); askQuestion()}
	else{
		let file = fs.createReadStream(path, {encoding: null});
		let name = pathmod.basename(path);
		const form = new FormData();
		form.append('path', to || name);
		form.append('file', file);
		takeOptions.headers = {...form.getHeaders(), };

		const req = http.request(takeOptions, res => {  
			res.on('data', d => {
				let data = JSON.parse(d);
				if(data.error){
					console.log(`\tError: ${data.message}`);
				}else{
					console.log(`\tsrv> ${data.message}`);
				}
				askQuestion();
			})
		})
			
		req.on('error', error => {
			console.error(error)
		})
	
		form.pipe(req);
	}
}

function want(from, to){
	if(from == null){
		console.log('\tError: need a filepath to request');
	}
	const req = http.request(wantOptions, res => {  
		let form = new formidable.IncomingForm();
		form.parse(res, (err, fields, files) => {
			if (err) {
				console.error(err);
			}else if(fields.error == 'true'){
				console.log(`\tError: ${fields.message}`);
				return askQuestion();
			}else{
				if(files.file == null) {
					console.log(`\tError: no file received`);
					return askQuestion();
				}
				fs.copy(files.file.filepath, DEFAULT_DIRECTORY + (to || fields.name), err => {
					if(err){
						console.log('\tError: problem copying file to destination');
					}else{
						fs.unlinkSync(files.file.filepath);
					}
					askQuestion();
				})
			}
		})
	});
		
	req.on('error', error => {
		console.error(error)
	})

	req.write(JSON.stringify({path: from}))
	req.end();
}

askQuestion();