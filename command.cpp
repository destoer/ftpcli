#include "command.h"
#include <stdlib.h>

namespace ftp  {
	
// returns the error code out of a repsonse
// to a ftp command as an int
int getErrorCode(std::string reply) {
	
	// the error code is contained in the first 3 characters
	// this makes things easy
	return atoi(reply.substr(0,3).c_str());
}

};