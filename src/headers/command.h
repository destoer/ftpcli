#pragma once

#include <string>
#include <stdbool.h>
#include "DataClient.h"
#include "Connection.h"
#include "CommandClient.h"


namespace ftp  {
	int getErrorCode(std::string reply); // short enough to inline
	bool isError(int code);
	DataClient initPasv(CommandClient client); // inits a pasv client for us
}