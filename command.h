#pragma once

#include <string>
#include <stdbool.h>


namespace ftp  {
	int getErrorCode(std::string reply); // short enough to inline
	bool isError(int code);
}