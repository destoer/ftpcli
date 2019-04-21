#include "DataClient.h"

// inits structs for the different ftp modes that can be used 
// to initialize the DataClient class
namespace ftp {
	
	DataClient_struct initActiveStruct(const char *port);
	DataClient_struct initPasvStruct(const char *ip, const char *port);
};