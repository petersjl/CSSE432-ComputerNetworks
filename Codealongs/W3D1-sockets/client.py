import socket
import sys

def client_program():
	if(len(sys.argv) != 3):
		print('Usage, python client.py <ServerIP> <ServerPort>')
		sys.exit()

	port = int(sys.argv[2])
	server_ip = socket.gethostbyname(sys.argv[1])

	server_addr = (server_ip, port)

	print(f'Server IP: {server_ip}')

	client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	client_socket.connect(server_addr)
	while True:
		message = input("-> ")

		if message.lower().strip() == 'exit': break

		client_socket.send(message.encode())

		data = client_socket.recv(1024).decode()
		print(f'Received from server: {str(data)}')
	
	client_socket.close()

if __name__ == '__main__':
	client_program()