// inits connection for the data client 
// either pasv mode or active mode
#include "headers/DataClient.h"
#include <iostream>

namespace ftp {

// inits the connection struct for active mode
DataClient_struct initActiveStruct(const char *port) {

	DataClient_struct c;
	SOCKET listenSocket; // used once to get the ftp server to connect the client 
						 // as we have one peer past this we are done with this socket 
						 // and it can be shut down
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
	listenSocket = socket(c.result->ai_family, c.result->ai_socktype, c.result->ai_protocol);
	if(listenSocket == INVALID_SOCKET)
	{
		std::cerr << "socket failed: " << WSAGetLastError() << std::endl;
		freeaddrinfo(c.result);
		WSACleanup();
		exit(1);
	}
	
	
	// bind the socket
	rc = bind(listenSocket,c.result->ai_addr, (int)c.result->ai_addrlen);
	if(rc == SOCKET_ERROR)
	{
		std::cerr << "bind failed: " << WSAGetLastError() << std::endl;
		freeaddrinfo(c.result);
		closesocket(listenSocket);
		WSACleanup();
		exit(1);
	}
	
	freeaddrinfo(c.result);
	
	
	
	
	// accept a client connection
	c.clientSocket = accept(listenSocket,NULL,NULL);
	if(c.clientSocket == INVALID_SOCKET)
	{
		std::cerr << "accept failed: " << WSAGetLastError() << std::endl;
		closesocket(listenSocket);
		WSACleanup();
		exit(1);
	}
	
	closesocket(listenSocket);
	return c;
}

// inits the connection struct for passive mode
DataClient_struct initPasvStruct(const char *ip, const char *port) {
	
	DataClient_struct c;
	
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
	return c;
}

};