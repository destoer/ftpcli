#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <tuple>
#include <locale>
#include <stdlib.h>
#include <limits.h>
#include "headers/ftp.h"

#undef _WIN32_WINNT
#define _WIN32_WINNT 0x501 

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>


// need the client class as the CommandClient class
// and another for the data port called DataClient
// which needs to listen and use some of our server code...
// to perform the data transfers


//https://en.wikipedia.org/wiki/List_of_FTP_commands
//https://en.wikipedia.org/wiki/List_of_FTP_server_return_codes


int main(int argc, char *argv[]) {
	
	// validate the args 
	if(argc != 3)
	{
		printf("Usage: %s <ip> <port>\n",argv[0]);
		exit(0);
	}
	

	
	printf("Attempting to connect to %s: %s\n",argv[1],argv[2]);
	

	
	ftp::CommandClient client{argv[1],argv[2]}; // init the connection of the ftp cli
	
	puts("Connected successfully!");
	
	
	// lets see if we can pull the header back 
	// speedtest.tele2.net
	// 220 (vsFTPd 3.0.3)\r\n
	
	// so commands will be terminated by \r\n
	// so we will read till we see the sequence or
	// we reach the end of our buffer
	
	// should get back the 220 code and be ready to auth
	std::string reply = client.recvCommand();
	std::cout << reply;
	
	int rc = ftp::getErrorCode(reply);
	
	if(rc != 220)
	{
		std::cerr << "[DEBUG] did not get a successfully completion reply\n";
	}
	
	

	
	// authenticate to the ftp server
	std::cout << "username: ";
	std::string username;
	std::cin >> username;
	
	
	client.sendCommand("USER " + username);
	
	
	
	// should get back the 331 code for req password
	reply = client.recvCommand();
	std::cout << reply;
	
	if(ftp::isError(ftp::getErrorCode(reply))) {
		std::cerr << "[DEBUG] username was not ok or another error occurred!\n";
	}
	
	std::cout << "password: ";
	std::string password; // could use get line and save the allocation...
	std::cin >> password;
	
	client.sendCommand("PASS " + password);
	
	std::cout << client.recvCommand();
	
	
	// ok we are now authenticated the user will now being sending commands 
	// lets configure the data port with the PORT command behind the scenes
	
	
	// for now we will user PASV to test the data send protocol
	// we will hoever test the PORT command locally
	
	// main ftp loop
	
	// as we have used cin with the stream operator we need to reset its state 
	// before we make calls to getline
	std::cin.clear();
	std::cin.ignore();
	
	std::string command;  // outside the loop as we dont want the constructor called 500 times
	std::string arg;
	for(;;) { // main command loop
		std::cout << "? ";
		std::getline(std::cin,command); // allow spaces

		
		
		// cool now we get just the prefix
		size_t pos = command.find(" ");
		
		
		if(pos != std::string::npos) {
			arg = command.substr(pos); // get the arg
			command = command.substr(0,pos); // strip after it so we have just the command
		} else { // no arg
			arg = "";
		}
		
		// now its been parsed ignore case in the command
		for(auto& x : command) { // take ref so we can modify it
 			x = std::tolower(x);
		}
		
		// real command ftp understands is LIST
		if(command == "dir") {  

			
			// however before we get the dir listing we need to setup the data channel
			// with PASV for this we need to send a command for pasv get the port back
			// and then make a new tempoary client and connect on it
			ftp::PassiveClient data_client = ftp::initPasv(client);
		
			client.sendCommand("LIST" + arg);
			std::string resp = client.recvCommand(); // check the error code in here too
			
			std::cout << resp;
			
			if(ftp::isError(ftp::getErrorCode(resp))) { // dir was not found so dont even try receiving any data
				continue;
			}
			
			std::string data;
			while(data_client.toRecv()) { // while the server hasnt shut the socket off
				if(data_client.recvAscii(data)) { // if read out correctly
					std::cout << data;
				}
			}
			
			// should check the error code to see it worked fine
			// for when we gui it but it aint useful for a cli 		
			std::cout << client.recvCommand();
		}
		
		// for get command we should specify type i for binary 
		// and just read out the file and straight up dump the binary data
		else if(command == "get") {
		
			client.sendCommand("TYPE I"); // switch to binary transfer
			// may need to switch back to ascii for the dir send
			
			std::string resp = client.recvCommand();
			
			std::cout << resp;
			
			// should handle this when its gui 
			// but its up to the user to decide in cli 
			// we will just prevent the send from going further for now
			if(ftp::isError(ftp::getErrorCode(resp))) {
				continue;
			}
	
			// setup the data connection
			ftp::PassiveClient data_client = ftp::initPasv(client);
			
			
			// now request the file 
			client.sendCommand("RETR" + arg);
			
			resp = client.recvCommand();
			
			std::cout << resp;
			
			// check that the file is valid
			// 550 is filesystem error
			if(ftp::getErrorCode(resp) == 550) {	
				// in this case it will give us some info why 
				// the file access failed 
				// we need to read until we get another 550 code 
				resp = client.recvCommand();
				std::cout << resp;
				while(resp.substr(0,3) != "550") {
					resp = client.recvCommand();
					std::cout << resp;
				}
				continue;
			}
			
			// receive the file
			int len = data_client.recvFile(arg);
			
			resp = client.recvCommand();
			std::cout << resp;
			
			if(!ftp::isError(ftp::getErrorCode(resp))) {
				std::cout << "Received: " << len << " bytes!\n";
			}
		}
		
		
		// quit out the program
		else if(command == "quit" || command == "bye") {
			client.sendCommand("QUIT"); // this is the actual command the server recognizes
			std::cout << client.recvCommand();
			return 0; // end the program
		}
		
		// default just send and recv
		// note as we have no fully implemented every command 
		// we will leave this for now 
		// but just have it print that it is an invalid command 
		// when our implementation is complete
		else {
			std::cerr << "[DEBUG] unknown command!\n";
			client.sendCommand(command+arg);
			std::cout << client.recvCommand();		
		}
	}
	
	
}