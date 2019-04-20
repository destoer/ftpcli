#pragma once

#undef _WIN32_WINNT
#define _WIN32_WINNT 0x501 

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdbool.h>
#include <string>

// holds the socket and connection info 
// for data being sent over the data port
typedef struct {
	struct addrinfo *result;
	struct addrinfo *ptr;
	struct addrinfo hints;
	SOCKET clientSocket;
	SOCKET listenSocket; // <-- required for the data port as we listen for connections in active mode
	WSADATA wsaData;
} DataClient_struct;



namespace ftp {
	
class DataClient // base class for Active and Passive Client
{
public: // implements the data send and recv funcitons
	
	bool recvAscii(std::string &data);
	int recvFile(std::string filename);
	bool toRecv(void) const; // true if more is to read  otherwhise false 

	// internal struct to hold socket info 
	// will be initialized by the dervied classes
	DataClient_struct c;
	
	
private:
	bool transfer_state = true; // true when client initalized
								// we can do this as only one data transfer is done 
								// before it drops out of scope
								// and we have to setup a new cli for the next 
								// transfer that will allocated to a different port 
	
};
	
	
	
};