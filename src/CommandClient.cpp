#include "headers/CommandClient.h"
#include "headers/net.h"
#include <iostream>
#include <utility>
#include <regex>


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

std::pair<std::string,std::string> CommandClient::initPasv(std::string pasv_reply) {
	
	// cool now we get something like this back 192,168,1,16,191,60
	// we need to pull each token behind hte commas
	// for the first 4 we use it to constuct the ip 
	// and the last we use it to pull the port 
	// where the port is the last two
	// with the 191 being the high byte and the 0x60 being the low byte
	

	// to pull this out we will replace the first 4 commas with .
	// get the ip from up to the 4 .
	// then pull the last section with rfind --> 191,60
	// then finally pull the port
	
	// get just the bit we need out of the command reply
	// 192,168,1,16,191,60 <-- regex below should pull this
	std::regex r("((\\d{1,3},){5})\\d*");
	std::smatch m;
	
	std::string tmp;
	if(std::regex_search(pasv_reply,m,r)) {
		tmp = pasv_reply.substr(m.position(), m.length());
	}
		
	else { // should except as this should not fail
		std::cerr << "invalid address!" << std::endl;
		exit(1);
	}
	
	
	// for the first 4 replace ',' with '.'
	size_t index = 0; 
	for(int i = {0}; i <= 3; i++) {
		index = tmp.find(",",index);
		if(index == std::string::npos) { 
			break;
		}
		tmp[index++] = '.';
	}
	size_t pos3 = tmp.rfind("."); // get the last dot where the ip ends
	std::string ip = tmp.substr(0,pos3); // and finally pull the ip
	
	// remove the ip from the string so we just have port part
	tmp = tmp.substr(pos3+1);
	
	// delimit low and high and convert the ascii to int
	pos3 = tmp.find(",");

	// both of these should hadnle the exception and either panic 
	// or retry the transfer up to a point

	uint8_t low, high;
	try {
		high = std::stoi(tmp.substr(0,pos3));
		low = std::stoi(tmp.substr(pos3+1));
	} catch(std::exception& e) {
		std::cout << "Failed to convert port to string!: " << e.what() 
			<< std::endl;;
		exit(1);
	}
	
	std::string port = std::to_string((high << 8) | low); // finally build the port	
	
	return std::make_pair(ip,port);	
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
	int rc = net::checkedSend(con.clientSocket,command.c_str(),len);
}


std::string CommandClient::recvCommand() {
	
	char buffer[512] = {0};
	
	bool success = false;
	

	// read one byte first and start at one 
	// so we cant read out of bounds
	int rc = net::checkedRecv(con.clientSocket,buffer,1);
		
	if(rc == 0) { // command socket has closed!?
		std::cerr << "server command channel closed unexpectedadly!" << std::endl;
		exit(1);
	}
	
	
	for(int i{1}; i < 512; i++)
	{
		// read one byte
		int rc = net::checkedRecv(con.clientSocket,&buffer[i],1);	
		
		if(rc == 0) { // command socket has closed!?
			std::cerr << "server command channel closed unexpectedadly: " << std::endl;
			exit(1);
		}
		
		if(buffer[i] == '\n' && buffer[i-1] == '\r')
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
	int rc = WSAStartup(MAKEWORD(2,2), &con.wsaData);
	if(rc != 0) {
		std::cerr << "WSAStartup failed: " << rc << std::endl;
		exit(1);
	}
	
	ZeroMemory(&con.hints,sizeof(con.hints));
	con.hints.ai_family = AF_UNSPEC;
	con.hints.ai_socktype = SOCK_STREAM;
	con.hints.ai_protocol = IPPROTO_TCP;
	
	//resolve server address and port
	rc = getaddrinfo(ip,port,&con.hints, &con.result);
	if(rc != 0) {
		std::cerr << "getaddrinfo failed: " << rc << std::endl;
		WSACleanup();
		exit(1);
	}
	
	// attempt connection until one succeeds
	for(con.ptr=con.result; con.ptr != NULL; con.ptr=con.ptr->ai_next) {
		con.clientSocket = socket(con.ptr->ai_family, con.ptr->ai_socktype, con.ptr->ai_protocol);
		if(con.clientSocket == INVALID_SOCKET) {
			std::cerr << "socket failed: " << WSAGetLastError() << std::endl;
			WSACleanup();
			exit(1);
		}
		

		
		// connect to the server
		rc = connect(con.clientSocket,con.ptr->ai_addr, (int)con.ptr->ai_addrlen);
		if(rc == SOCKET_ERROR) {
			closesocket(con.clientSocket);
			con.clientSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}
	
	freeaddrinfo(con.result);
	
	if(con.clientSocket == INVALID_SOCKET) {
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