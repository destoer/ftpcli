#include <string>
#include <fstream>
#include <stdbool.h>
bool fileExists(std::string filename) {
	std::fstream fp{filename};
	if(fp.good()) {
		return true;
	}
	return false;
}