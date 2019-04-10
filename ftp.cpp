#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <tuple>
#include <stdlib.h>
#include <limits.h>
#include "ftp.h"

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
	
	if(ftp::getErrorCode(reply) != SEND_PASS) {
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
	std::cin.clear();
	std::cin.ignore();
	std::string command;  // outside the loop as we dont want the constructor called 500 times
	std::string arg;
	for(;;) {
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
		
		// real command ftp understands is LIST
		if(command == "dir") {  // do this first to practice using the data port
			// need to split the command to get the directory
			// and replace dir with list 
			
			
			

			// however before the do this we need to setup the data channel
			// with PASV for this we need to send a command for pasv
			// get the port back
			// and then make a new tempoary client and connect on it
			client.sendCommand("PASV");
			std::string pasv_reply = client.recvCommand();
			
			std::cout << pasv_reply;
			
			// should check that pasv initalized properly
			// but im not sure how to best handle the error
			
			std::string ip;
			std::string port;
			
			std::tie(ip,port) = client.initPasv(pasv_reply);
			
			std::cout << "Connecting on " << ip << ":" << port << "\n";
			
			
			// init the passive client and request the directory
			ftp::PassiveClient data_client{ip,port}; // <-- one we have atm is the passive client wrongly named but change it later
		
			client.sendCommand("LIST" + arg);
			std::string resp = client.recvCommand(); // check the error code in here too
			
			std::cout << resp;
			
			if(ftp::isError(ftp::getErrorCode(resp))) { // dir was not found so dont even try receiving any data
				continue;
			}
			
			
			std::string data;
			while(data_client.toRecv()) { // no idea how to determine when data sends are done,,
				if(data_client.recvAscii(data)) { // if read out correctly
					std::cout << data;
				}
			}
			std::cout << client.recvCommand();	// should check the error code to see it worked fine
												// for when we gui it
												// but it aint useful for a cli 
		}
		
		// default just send and recv
		else {
			std::cerr << "[DEBUG] unknown command!\n";
			client.sendCommand(command);
			std::cout << client.recvCommand();		
		}
		

	}
	
	
}