const http = require('http');
const readline = require('readline');

const rl = readline.createInterface({
	input: process.stdin,
	output: process.stdout
});

if (process.argv[3] == null){
	console.log("Usage: node clinet.js host port.");
	process.exit();
}

const options = {
  hostname: process.argv[2],
  port: process.argv[3],
  path: '/',
  method: 'POST',
  headers: {
    'Content-Type': 'application/json'
  }
};

console.log(`Talking to host: ${options.hostname} on port ${options.port}\nType ';;;' to exit\n`);

function askQuestion(){
	rl.question("You> ", function (p) {
	if (p == ";;;"){
		console.log('Bye!');
		process.exit();
	}else{
		const req = http.request(options, res => {  
			res.on('data', d => {
			  	console.log(`Srv> ${JSON.parse(d).message}`);
				askQuestion();
			})
		})
		  
		req.on('error', error => {
			console.error(error)
		})

		req.write(JSON.stringify({message: p}));
		req.end();
	}
	})
}

askQuestion();