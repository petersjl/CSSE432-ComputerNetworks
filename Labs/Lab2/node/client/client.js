const http = require('http');
const readline = require('readline');
const fs = require('fs');

const rl = readline.createInterface({
	input: process.stdin,
	output: process.stdout
});

const DEFAULT_DIRECTORY = __dirname + "/store";

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
		'Content-Type': 'application/json'
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
			case "ls": 
			case "list": {
				list(parts[1] || "/");
			}break;
			case "iwant" : {
				if(!parts[1]) {
					console.log("\tUsage: iWant <file path on server> (destination on client)\n");
					askQuestion();
				}
				want(parts[1], parts[2] || DEFAULT_DIRECTORY);
			}break;
			case "utake" : {
				if(!parts[1]) {
					console.log("\tUsage: uTake <file path on client> (destination on server)\n");
					askQuestion();
				}
				take(parts[1], parts[2] || DEFAULT_DIRECTORY);
			}break;
			default : {
				console.log("Available commands:\nls (path)\niWant <file path on server> (destination on client)\nuTake <file path on client> (destination on server)\n");
				askQuestion();
			}
		}
	}
	})
}

function list(path){
	console.log(path);
	const req = http.request(listOptions, res => {  
		res.on('data', d => {
			let data = JSON.parse(d);
			if(data.error){
				console.log("Error: path not found");
			}else{
				console.log(`${path}> ${data.message}`);
			}
			askQuestion();
		})
	})
		
	req.on('error', error => {
		console.error(error)
	})

	req.write(JSON.stringify({path: path}));
	req.end();
}

function take(from, to){

}

function want(from, to){

}

askQuestion();