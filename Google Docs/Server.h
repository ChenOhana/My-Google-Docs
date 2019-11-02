#pragma once

#include <WinSock2.h>
#include <Windows.h>
#include <exception>
#include <string>
#include <deque>
#include <thread>
#include <mutex>
#include "Helper.h"

using std::cout;
using std::endl;
using std::deque;
using std::thread;
using std::pair;
using std::mutex;
using std::fstream;

class Server
{
public:
	Server(); //constructor
	~Server(); //destructor
	void serve(int port); //start server

private:
	void accept(); //accept client
	void clientHandler(SOCKET clientSocket); //handle client communication
	string getFileContent(string filePath); //return file content
	void send101Message(string content); //send 101 message to all clients
	void deleteUser(SOCKET clientSocket, string name); //delete user
	deque<pair<SOCKET, string>> _users;
	SOCKET _serverSocket;
};