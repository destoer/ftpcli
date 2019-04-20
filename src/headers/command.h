#pragma once

#include <string>
#include <stdbool.h>
#include "CommandClient.h"
#include "PassiveClient.h"


namespace ftp  {
	int getErrorCode(std::string reply); // short enough to inline
	bool isError(int code);
	PassiveClient initPasv(CommandClient client); // inits a pasv client for us
}