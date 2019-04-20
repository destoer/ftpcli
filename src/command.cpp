#include <string>
#include <locale>
#include <iostream>
#include "headers/command.h"

namespace ftp  {
	
// returns the error code out of a repsonse
// to a ftp command as an int
int getErrorCode(std::string reply) {
	
	std::string reply_code = reply.substr(0,3);
	
	// 777 code is an error code
	// but also one ftp will never throw
	// so used to indicate no code returned
	for(auto x : reply_code) {
		if(!std::isdigit(x)) return 777;
	}
	
	// the error code is contained in the first 3 characters
	// this makes things easy
	return std::stoi(reply_code);
}

// might be better to just pass a string and have it call getErrorCode for us 
// if we dont care what the error is
bool isError(int code) {
	return code > 400; // 400 and 500 range are errors
}

// will intialize a pasv connection to the server for us
PassiveClient initPasv(CommandClient client) {
	// setup the data connection
	client.sendCommand("PASV");
	std::string resp = client.recvCommand();
	
	std::cout << resp;
	
	// check that pasv initalized properly
	if(isError(getErrorCode(resp))) {
		std::cout << "Pasv unhandled error!: "; // should except by here
		exit(1);
	}
	
	std::string ip;
	std::string port;
	std::tie(ip,port) = client.initPasv(resp);	
	
	std::cout << "Connecting on " << ip << ":" << port << "\n";				
	PassiveClient data_client{ip,port};	
	
	
	return data_client;
}


};