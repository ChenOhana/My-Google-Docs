/*********************************
* Class: MAGSHIMIM CPP 2		 *
* Week: 13        				 *
*                   			 *
**********************************/

#pragma comment (lib, "ws2_32.lib")

#include "Server.h"
#include "WSAInitializer.h"

int main()
{
	try
	{
		cout << "Starting..." << endl;
		WSAInitializer wsaInit;
		Server myServer;
		myServer.serve(1234);
	}
	catch (std::exception& e)
	{
		std::cout << "Error occured: " << e.what() << std::endl;
	}
	system("pause");

	return 0;
}