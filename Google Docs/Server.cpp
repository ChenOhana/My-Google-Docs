#include "Server.h"

mutex mt;

/*
Constructor will create server socket.
Input: None
*/

Server::Server()
{
	// this server use TCP. that why SOCK_STREAM & IPPROTO_TCP
	// if the server use UDP we will use: SOCK_DGRAM & IPPROTO_UDP
	_serverSocket = socket(AF_INET,  SOCK_STREAM,  IPPROTO_TCP); 

	if (_serverSocket == INVALID_SOCKET)
	{
		throw std::exception(__FUNCTION__ " - socket");
	}
}

//Destructor

Server::~Server()
{
	try
	{
		// the only use of the destructor should be for freeing 
		// resources that was allocated in the constructor
		closesocket(_serverSocket);
	}
	catch (...) {}
}

/*
Function will bind the client with the server and start communication.
Input: port
Output: None
*/

void Server::serve(int port)
{
	struct sockaddr_in sa = { 0 };
	
	sa.sin_port = htons(port); // port that server will listen for
	sa.sin_family = AF_INET;   // must be AF_INET
	sa.sin_addr.s_addr = INADDR_ANY;    // when there are few ip's for the machine. We will use always "INADDR_ANY"

	// again stepping out to the global namespace
	// Connects between the socket and the configuration (port and etc..)
	if (bind(_serverSocket, (struct sockaddr*)&sa, sizeof(sa)) == SOCKET_ERROR)
		throw std::exception(__FUNCTION__ " - bind");
	cout << "binded" << endl;
	// Start listening for incoming requests of clients
	if (listen(_serverSocket, SOMAXCONN) == SOCKET_ERROR)
		throw std::exception(__FUNCTION__ " - listen");
	cout << "listening..." << endl;

	while (true)
	{
		// the main thread is only accepting clients 
		// and add then to the list of handlers
		cout << "accepting client..." << endl;
		accept();
	}
}

/*
Function will accept the client, create thread with clientHandler function and detach it.
Input: None
Output: None
*/

void Server::accept()
{
	// notice that we step out to the global namespace
	// for the resolution of the function accept

	// this accepts the client and create a specific socket from server to this client
	SOCKET client_socket = ::accept(_serverSocket, NULL, NULL);

	if (client_socket == INVALID_SOCKET)
	{
		throw std::exception(__FUNCTION__);
	}
	
	cout << "Client accepted !" << endl;
	thread t(&Server::clientHandler, this, client_socket);
	t.detach();
	cout << "ADDED new client " << client_socket << ", userName to clients list" << endl;
}

/*
Function will handle the communication of the server with the client.
Input: clientSocket
Output: None
*/

void Server::clientHandler(SOCKET clientSocket)
{
	try
	{
		static int position = 0;
		string response = Helper::getStringPartFromSocket(clientSocket, 100);
		int size = atoi(response.substr(3, 2).c_str());
		string name = response.substr(5, size);
		_users.push_back(pair<SOCKET, string>(clientSocket, name));
		string currUser = _users[0].second;
		string nextUser = _users[1 % _users.size()].second;
		position++;
		string content = getFileContent("shared_doc.txt");
		Helper::sendUpdateMessageToClient(clientSocket, content, currUser, nextUser, position);
		
		int typeCode = 0;
		
		while (typeCode != 208)
		{
			response = Helper::getStringPartFromSocket(clientSocket, 1024);
			cout << "Recieved update message from current client" << endl;
			typeCode = atoi(response.substr(0, 3).c_str());

			if (typeCode != 208)
			{
				size = atoi(response.substr(3, 5).c_str());
				content = response.substr(8, size);
				mt.lock();
				fstream file("shared_doc.txt", std::ios::out | std::ios::trunc);
				file << content;
				file.close();
				mt.unlock();

				if (typeCode == 207) //move head of queue to end of queue
				{
					cout << "Recieved finish message from current client" << endl;
					pair<SOCKET, string> temp = _users.front();
					mt.lock();
					_users.pop_front();
					_users.push_back(temp);
					mt.unlock();
				}
				cout << "Send update message to all clients" << endl;
				send101Message(content);
			}
		}
		cout << "Recieved exit message from client" << endl;
		cout << "REMOVED " << clientSocket << ", userName from clients list" << endl;
		deleteUser(clientSocket, name);
		send101Message(content);
		closesocket(clientSocket);
		position--;
	}
	catch (const std::exception& e)
	{
		closesocket(clientSocket);
	}
}

/*
Function will open file and return its content.
Input: filePath
Output: content
*/

string Server::getFileContent(string filePath)
{
	fstream file(filePath);
	string line, content;

	while (getline(file, line))
	{
		content += line + "\n";
	}
	file.close();

	return content;
}

/*
Function will send 101 message to all connected clients.
Input: file content
Output: None
*/

void Server::send101Message(string content)
{
	for (int i = 0; i < _users.size(); i++)
	{
		Helper::sendUpdateMessageToClient(_users[i].first, content, _users[0].second, _users[1 % _users.size()].second, i + 1);
	}
}

/*
Function will delete user from _users deque.
Input: clientSocket, name
Output: None
*/

void Server::deleteUser(SOCKET clientSocket, string name)
{
	auto it = std::find(_users.begin(), _users.end(), pair<SOCKET, string>(clientSocket, name));

	if (it != _users.end())
	{
		_users.erase(it);
	}
}