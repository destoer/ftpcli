#include "CommandClient.h"
#include <iostream>



// Client constructor will setup the connection struct
// and init the connection to the ftp server 
// at the passed ip and port

namespace ftp {

CommandClient::CommandClient(const char *ip, const char *port) {
	
	
	// for now it just directly calls it 
	// but the real reason we want to do this
	// is for error propagation later
	// so they can retype the ip if it failed etc
	initConnection(ip,port);
	
	
}


// Send a command to the ftp server

void CommandClient::sendCommand(std::string command) {
	
	int len = command.length();
	
	if(len < 0) {
		std::cerr << "[DEBUG] attempted to send empty command!\n";
	}
	
	command += "\r\n"; // terminate the command

	len += 2; // update the length for our termination 

	
	// send the actual buffer
	int rc = send(c.clientSocket,command.c_str(),len,0);
	if(rc == SOCKET_ERROR)
	{
		std::cerr << "send failed: " << WSAGetLastError() << std::endl;
		//closesocket(c.clientSocket);
		//WSACleanup();
		//throw runtime_error("Send failed: " + WSAGetLastError());
		return;
	}
	
}


std::string CommandClient::recvCommand() {
	
	char buffer[512] = {0};
	
	bool success = false;
	
	
	// should just read once first so we dont have to check the i > 0
	for(int i{0}; i < 512; i++)
	{
		// read one byte
		int rc = recv(c.clientSocket,&buffer[i],1,0);
		
		if( rc < 0 )
		{
			std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
			//closesocket(c.clientSocket);
			//WSACleanup();
			//throw runtime_error("recv failed" + WSAGetLastError()); (should throw our own well defined error)
			return std::string(""); // same as below
		}	
		
		
		
		if(buffer[i] == '\n' && buffer[i-1] == '\r' && i > 0)
		{
			success = true;
			break;
		}
	}

	if(!success) {
		std::cerr << "[DEBUG] recving command exceeded the buffer length: " << buffer << "\n";
		return std::string(""); // should raise an exception or return a well defined error
							// that ftp cannot send as a command
	}
	
	return std::string(buffer);
	
	
}

// destructor
CommandClient::~CommandClient() {
	WSACleanup(); // cleanup our sockets
}

// attempt to connect to the a ftp server on a specified ip and port
void CommandClient::initConnection(const char *ip, const char *port) {
	int rc = WSAStartup(MAKEWORD(2,2), &c.wsaData);
	if(rc != 0) {
		std::cerr << "WSAStartup failed: " << rc << std::endl;
		exit(1);
	}
	
	ZeroMemory(&c.hints,sizeof(c.hints));
	c.hints.ai_family = AF_UNSPEC;
	c.hints.ai_socktype = SOCK_STREAM;
	c.hints.ai_protocol = IPPROTO_TCP;
	
	//resolve server address and port
	rc = getaddrinfo(ip,port,&c.hints, &c.result);
	if(rc != 0) {
		std::cerr << "getaddrinfo failed: " << rc << std::endl;
		WSACleanup();
		exit(1);
	}
	
	// attempt connection until one succeeds
	for(c.ptr=c.result; c.ptr != NULL; c.ptr=c.ptr->ai_next) {
		c.clientSocket = socket(c.ptr->ai_family, c.ptr->ai_socktype, c.ptr->ai_protocol);
		if(c.clientSocket == INVALID_SOCKET) {
			std::cerr << "socket failed: " << WSAGetLastError() << std::endl;
			WSACleanup();
			exit(1);
		}
		

		
		// connect to the server
		rc = connect(c.clientSocket,c.ptr->ai_addr, (int)c.ptr->ai_addrlen);
		if(rc == SOCKET_ERROR) {
			closesocket(c.clientSocket);
			c.clientSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}
	
	freeaddrinfo(c.result);
	
	if(c.clientSocket == INVALID_SOCKET) {
		std::cerr << "unable to connect to server" << std::endl;
		WSACleanup();
		exit(1);
	}
	
	
	/*unsigned long mode = 1;
		
	// make the socket non blocking (need to figure out how to use select())
	// probably required for correct operation
	rc = ioctlsocket(c->clientSocket,FIONBIO, &mode);		

	// failure to set the socket mode
	if(rc != 0) {
		std::cerr << "ioctlsocket failed with error: " << rc << std::endl;
		WSACleanup();
		exit(1);
	}*/
	
}

}