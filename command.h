#pragma once

#include <string>


namespace ftp  {
	int getErrorCode(std::string reply); // short enough to inline
}