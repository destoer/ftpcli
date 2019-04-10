#include <string>
#include "command.h"

namespace ftp  {
	
// returns the error code out of a repsonse
// to a ftp command as an int
int getErrorCode(std::string reply) {
	
	// the error code is contained in the first 3 characters
	// this makes things easy
	return std::stoi(reply.substr(0,3));
}


bool isError(int code) {
	return code > 400; // 400 and 500 range are errors
}

};