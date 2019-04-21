#pragma once


#undef _WIN32_WINNT
#define _WIN32_WINNT 0x501 

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

#include <string>


// holds the socket and connection info 
// for data being sent over the command port
// could reuse the dataclient one but this 
// makes its purpose more clear
typedef struct {
	struct addrinfo *result;
	struct addrinfo *ptr;
	struct addrinfo hints;
	SOCKET clientSocket;
	WSADATA wsaData;
} CommandClient_struct;


// ftp command client class
// used for communicating over the command port

namespace ftp {


class CommandClient
{
public:
	// constructor
	CommandClient(const char *ip, const char *port); // should have support for hostnames too but this is fine for now
	~CommandClient(); // destructor

	// this need to return well defined exceptions saying if the send /recv has failed...
	void sendCommand(std::string command);
	std::string recvCommand(); // recv a command from the server
	std::pair<std::string,std::string> initPasv(std::string);
	
	
private:
	// internal helper for initializing the connection
	void initConnection(const char *ip, const char *port); 
	
	
	
	// the internal connection struct
	CommandClient_struct con; // internal struct to hold socket info
};

};