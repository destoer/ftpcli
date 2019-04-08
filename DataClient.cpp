// functions for sending and receiving data over the data port will go here
#include "DataClient.h"
#include <iostream>

namespace ftp {

// will throw an exception if there is an attempt to read out of the state
// while it is false
// besides setting the var when the tranfser is done 
// this is identical to the command recv function 

bool DataClient::recvAscii(std::string &data) {
	
	if(!transfer_state) { } // should  except here
	
	char buffer[512] = {0};
	
	bool success = false;
	
	
	// should just read once first so we dont have to check the i > 0
	// and a termination has to be 2 chars so it will never be just one
	for(int i{0}; i < 512; i++)
	{
		// read one byte (potentially could be optimised by reading all availiable bytes)
		int rc = recv(c.clientSocket,&buffer[i],1,0);
		
		if(rc == 0) { // socket has been closed and data transfer ended
			transfer_state = false; // indicate we are done
			return false;
		}

		
		else if( rc < 0 )
		{
			std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
			//closesocket(c.clientSocket);
			//WSACleanup();
			//throw runtime_error("recv failed" + WSAGetLastError()); (should throw our own well defined error)
			return false; // should except by here
		}	
		

		
		
		
		if(buffer[i] == '\n' && buffer[i-1] == '\r' && i > 0)
		{
			success = true;
			break;
		}
	}

	if(!success) {
		// maybye we should just alloc a larger buffer up to a point but ignore for now
		std::cerr << "[DEBUG] ascii exceeded the buffer length: " << buffer << "\n";
		return false; // should raise an exception or return a well defined error
					// that ftp cannot send as a command
	}
	
	data = std::string(buffer);
	return true;	
}

bool DataClient::toRecv(void) const {
	return transfer_state;
}


};