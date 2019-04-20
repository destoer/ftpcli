#pragma once
#include "DataClient.h"


namespace ftp {
	
class ActiveClient: public DataClient
{
public:
	// constructor
	ActiveClient(std::string port); // should have support for hostnames too but this is fine for now
	~ActiveClient(); // destructor

private:

	// internal helper for initializing the connection
	void initConnection(const char *port); 
	
};
	
	
	
};