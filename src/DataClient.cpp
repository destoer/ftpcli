// functions for sending and receiving data over the data port will go here
#include "headers/DataClient.h"
#include "headers/constants.h"
#include "headers/net.h"
#include <iostream>
#include <fstream>

namespace ftp {

DataClient::DataClient(DataClient_struct connection) :con(connection) {}

DataClient::~DataClient() {
	if(con.clientSocket != INVALID_SOCKET) { // if null its allready shut 
									 // dont do it twice
		closesocket(con.clientSocket);
	}
}

// will throw an exception if there is an attempt to read out of the state
// while it is false
// besides setting the var when the tranfser is done 
// this is identical to the command recv function 

bool DataClient::recvAscii(std::string &data) {
	
	if(!transfer_state) { return false; } // should  except here
	
	char buffer[512] = {0};
	
	bool success = false;

	// read one byte out first and set index to 1 so we cant read out of bounds
	int rc = net::checkedRecv(con.clientSocket,buffer,1);
		
	if(rc == 0) { // socket has been closed and data transfer ended
		transfer_state = false; // indicate we are done
		return false;
	}

	// start at one so we dont read out of bounds
	for(int i{1}; i < 512; i++)
	{
		int rc = net::checkedRecv(con.clientSocket,&buffer[i],1);
		
		if(rc == 0) { // socket has been closed and data transfer ended
			transfer_state = false; // indicate we are done
			return false;
		}
	

		// buffer must have alteast one thing in it 
		// so checking the buffer backwards cannot read out of bounds
		if(buffer[i] == '\n' && buffer[i-1] == '\r') {
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

// data is in binary mode so just outright read it off the buffer
int DataClient::recvFile(std::string filename) {
	char buf[4096];
	int i{0};
	
	std::fstream fp;
	fp.open(filename,std::ios::binary | std::ios::out);
	
	
	for(;;) {
		// recv until the conneciton is closed or the buffer is full
		int rc = net::checkedRecv(con.clientSocket,reinterpret_cast<char*>(&buf),4096,MSG_WAITALL);
	
		if(rc == 0) { // socket is closed and the file xfer is done 
			fp.close();
			return i;
		}
		
		else { // write the data out
			i += rc;
			fp.write(buf,rc);
		}
	}
}


int DataClient::sendFile(std::string filename) {
	const size_t SIZE = 4096;
	char buf[SIZE];
	int rc = 0;
	std::fstream fp{filename, std::ios::binary | std::ios::in};
	
	if(!fp) {
		std::cout << "File: " << filename << "not found!\n";
		return -1; // should return a #defined error code
				   // for when we fool "proof" this as a lib
				   // this is something that "should" actually fail 
				   // unlike a the sockets failing so we should not except for it
	}
	
	// get file size
	fp.seekg(0,std::ios::end);
	int len = fp.tellg();
	const int lengthOfFile = len;
	fp.clear();
	fp.seekg(0,std::ios::beg);
	
	while(len != 0) {
	
		if(len >= SIZE) {
			// read in the file and send 
			fp.read(buf,SIZE);
			rc = net::checkedSend(con.clientSocket,buf,SIZE);
			len -= SIZE;
		}
		
		// read remaining length
		else {
			fp.read(buf, len);
			rc = net::checkedSend(con.clientSocket,buf,len);
			len = 0;
		}	
	}
	closesocket(con.clientSocket);
	con.clientSocket = INVALID_SOCKET; // indicate we dont need to do it 
	return lengthOfFile;
}



};