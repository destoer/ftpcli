#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdexcept>

// defines light wrappers for send and recv that except 
// upon errors

namespace net {
	
class SocketError : public std::exception {

public:
	SocketError(std::string& msg, int err) : 
		error_code(err), error_message(msg) {}

	virtual ~SocketError() throw() {}

	virtual const char * what() const throw () {
		return error_message.c_str();
	}

	virtual int getErrorCode() const throw () {
		return error_code;
	}

	
protected:

	int error_code; // error code the socket returned
	std::string error_message; // message returned by WSAGetLastError()
	
};
	
	
	
	
int checkedSend(SOCKET s, const char *buf, int len, int flags = 0);
int checkedRecv(SOCKET s, char *buf, int len, int flags = 0);
	
};