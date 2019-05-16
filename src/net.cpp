#include "headers/net.h"

namespace net {
	
int checkedSend(SOCKET s, const char *buf, int len, int flags) {
	
	int rc = send(s,buf,len,flags);

	// throw an exception the socket has failed	
	if(rc < 0) {
		std::string error = "send failed: " + std::to_string(WSAGetLastError()) + "\n!";
		throw SocketError{error,rc};
	}	

	return rc;
}

int checkedRecv(SOCKET s, char *buf, int len, int flags) {

	int rc = recv(s,buf,len,flags);

	
	// throw an exception the socket has failed	
	if(rc < 0)  {
		// could be improved to get the string with a call to FormatMessage
		std::string error = "recv failed: " + std::to_string(WSAGetLastError()) + "\n!";
		throw SocketError{error,rc};
	}	
	
	// no error return the code
	return rc;
}
	
};