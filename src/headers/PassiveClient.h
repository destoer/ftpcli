// will be used later will implement its own destructor and constructor
// for the server implemenation
#pragma once
#include "DataClient.h"

namespace ftp {
	
class PassiveClient: public DataClient
{
public:
	// constructor
	
	// consider using move semantics with && is it may no longer be needed
	PassiveClient(std::string ip, std::string port); // should have support for hostnames too but this is fine for now
	~PassiveClient(); // destructor

private:

	// internal helper for initializing the connection
	void initConnection(const char *ip, const char *port); 
	
};
	
	
	
};