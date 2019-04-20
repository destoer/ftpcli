#include "headers/ActiveClient.h"
#include <iostream>



namespace ftp {

// PORT Command must be sent before this inits
// or we will have problems 
ActiveClient::ActiveClient(std::string port) {
		initConnection(port.c_str());
}

void ActiveClient::initConnection(const char *port) {

	int rc = WSAStartup(MAKEWORD(2,2), &c.wsaData);
	if(rc != 0) // non zero is a fatal error
	{
		std::cerr << "WSAStarted failed:" << rc << std::endl;
		exit(1); // fine to exit here instead of raising an exception
				// because the server will be in a useless state if the server
				// cant take incoming connections
	}
	
	ZeroMemory(&c.hints,sizeof(c.hints));
	c.hints.ai_family = AF_INET;
	c.hints.ai_socktype = SOCK_STREAM;
	c.hints.ai_protocol = IPPROTO_TCP;
	c.hints.ai_flags = AI_PASSIVE;
	
	// resolve the server address and port
	// hardcoded port
	rc = getaddrinfo(NULL,port, &(c.hints), &(c.result));
	if(rc != 0)
	{
		std::cerr << "getaddrinfo failed: " << rc << std::endl;
		WSACleanup();
		exit(1);
	}
	
	// create a socket connecting to the server
	c.listenSocket = socket(c.result->ai_family, c.result->ai_socktype, c.result->ai_protocol);
	if(c.listenSocket == INVALID_SOCKET)
	{
		std::cerr << "socket failed: " << WSAGetLastError() << std::endl;
		freeaddrinfo(c.result);
		WSACleanup();
		exit(1);
	}
	
	
	// bind the socket
	rc = bind(c.listenSocket,c.result->ai_addr, (int)c.result->ai_addrlen);
	if(rc == SOCKET_ERROR)
	{
		std::cerr << "bind failed: " << WSAGetLastError() << std::endl;
		freeaddrinfo(c.result);
		closesocket(c.listenSocket);
		WSACleanup();
		exit(1);
	}
	
	freeaddrinfo(c.result);
}
	
// destructor
ActiveClient::~ActiveClient() {
	//WSACleanup(); // cleanup our sockets (this should be called at the end)
				 // what we want to do is just close the socket..
	closesocket(c.clientSocket);
	closesocket(c.listenSocket); // server so listen sockets needs to go too	
}

};