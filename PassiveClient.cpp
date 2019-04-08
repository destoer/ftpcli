#include "PassiveClient.h"
#include <iostream>
#include <stdlib.h>

namespace ftp {

PassiveClient::PassiveClient(std::string ip, std::string port) {

	// bind on the port and wait for incoming connections
	// the port command should be sent just before this
	initConnection(ip.c_str(),port.c_str());
	
}


// attempt to connect to the a ftp server on a specified ip and port
void PassiveClient::initConnection(const char *ip, const char *port) {
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

// destructor
PassiveClient::~PassiveClient() {
	//WSACleanup(); // cleanup our sockets (this should be called at the end)
				 // what we want to do is just close the socket..
	closesocket(c.clientSocket); // just client socket as we aernt the server 
	
}

};

