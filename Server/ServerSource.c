/*-------------------------------------------------------------------------------------*/
/*|NAME: Jennie Martin****************************************************************|*/
/*|CLASS: CSC 3350 - Tindall**********************************************************|*/
/*|ASSIGNMENT: Lab 6- IPC Socket Talk- Server*****************************************|*/
/*|DESCRIPTION: Creates listening socket to connect to client program on open port****|*/
/*|*************and then the applications engage in a talk session via socket until***|*/
/*|*************exit is specified.****************************************************|*/
/*-------------------------------------------------------------------------------------*/
#include <stdio.h>    //required to use I/O
#include <conio.h>    //required for stdin listening function
#include <string.h>   //required for character string processing
#include <Winsock2.h> //required library to use sockets
#pragma comment(lib,"WS2_32")

#define buffsize 200 //maximum buffer size
#define HOSTNAMELEN 128 //host name length

/*-------------------------------------------------------------------*/
/*|FUNCTION PURPOSE: Engage in a talk session between server and****|*/
/*|******************client.****************************************|/
/*|PARAMETERS: socket***********************************************|*/
/*|RETURNS: nothing*************************************************|*/
/*-------------------------------------------------------------------*/
void talk(SOCKET skt);

int main(int argc, char *argv[]) //uses commandline
{
	WORD wVersionRequested;      //holds version of WSA
	WSADATA wsaData;             //holds WSA data
	SOCKET ConnSkt, ListSkt;     //socket for the server program to connect to client
	                             //socket for the server program to listen for client
	SOCKADDR_IN  Client, Listener;        //references the client, references the listener
	u_short  port = 0, actual_port = 0;  // Let Bind select a port # , actual port number used
	int result = 0, error = 0, hostname = 0, len = 0, ClientLen = 0; //result code from trying to get host table
	                                                               //holds bind result code
																   //size of client
																   //size of listener
	                                                               
	struct hostent  *host;                //points to host network address table

	char serverHostName[HOSTNAMELEN];     //character string to hold server host name

	//Initialize character buffer
	strcpy_s(serverHostName, HOSTNAMELEN, "");

	if ( argc != 1)
	{				 //if there is not 1 argument on the commandline 
					 //print the usage directions
		printf("  [Usage] Server.exe\n");
		return(0); //exit the program
	}

	//initialize socket package
	wVersionRequested = MAKEWORD(2, 2);  // Version 2.0

	if (WSAStartup(wVersionRequested, &wsaData) != 0) 
	{
		fprintf(stderr, "Process failed on WinSock startup\n");
		ExitProcess(0);
	};
	//gethostname of server
	result = gethostname(serverHostName, HOSTNAMELEN);
	if (result != 0) 
	{
		printf("The host name could not be gotten.\n");
		exit(1);
	}
	//Intro app code
	printf("#######################################\n");
	printf("TALK-SERVER: Jennie Martin Version\n");
	printf("#######################################\n");
	printf("\n Server Host: %s", serverHostName);
	

	//get server-host network address table
	host = gethostbyname(serverHostName);
	if (host == NULL)
	{
		fprintf(stderr, "Error getting server-host network address table!\n");
		ExitProcess(0);
	}
	//create a server listener socket for incoming client request
	ListSkt = socket(AF_INET, SOCK_STREAM, 0);

	//bind listener socket to server address and specific port
	// Configure Listener for this server host and port# 
	ZeroMemory(&Listener, sizeof(SOCKADDR_IN)); 
	Listener.sin_family = AF_INET; 
	Listener.sin_port   = htons((u_short) port);  // port # 
	CopyMemory(&Listener.sin_addr,     // Host     
	host->h_addr_list[0], host->h_length);

	result = bind(ListSkt, (const SOCKADDR *)&Listener, sizeof(Listener));
	if (result != 0) {
		error = GetLastError();
		fprintf(stderr, "Error occured in binding socket to specific host address/port:%d\n", error);
		ExitProcess(0);
	}
	//Get what port is being used 
	len = sizeof(Listener);
	getsockname(ListSkt, (SOCKADDR *)&Listener, &len);
	actual_port = ntohs(Listener.sin_port);

	//enable the listener incoming-event mechanism on listener socket
	listen(ListSkt, SOMAXCONN);

	printf("\n Server Port Number: %d", actual_port);
	printf("\n Listening...");

	//Wait to accept an incoming connection request from a client.  
	ClientLen = sizeof(SOCKADDR_IN);

	//Create a “client connection” socket for the specific connection: ConnSkt 
	ConnSkt = accept(ListSkt, (SOCKADDR *)&Client, &ClientLen);
	printf("accepted connection request\n");
	//close the listener socket
	closesocket(ListSkt);

	//Send/Recv data through the ConnSkt
	talk(ConnSkt);

	//When finished, close the ConnSkt
	closesocket(ConnSkt);
	
	//close the socket package 
	WSACleanup();

	return(0);
}

//function for "talking" between application
void talk(SOCKET skt)
{
	int flags = 0, nRead = 0, end = 0; // send/receive flags
									   //how many characters come in
								       //sentinal value to end
	char buffer[buffsize]; //line buffer for talk text
	strcpy_s(buffer, buffsize, ""); // initialize line buffer to nothing

	unsigned long  on = 1;
	ioctlsocket(skt, FIONBIO, &on);  // Set NB I/O on

	printf(">");
	while (end != 1) { //while the end flag is false

		if (_kbhit()) //if the user wrote a line from stdin
		{
			memset(buffer, 0, buffsize); // initialize line buffer to nothing
			// input entire line    . . .    . . . 
			gets_s(buffer, buffsize);
			buffer[strlen(buffer)] = '\n'; //add a newline to the end
			// Send the buffer
			send(skt, buffer, strlen(buffer), flags); // include newline
														  
			// remove \n from the end of the line before checking for exit condition
			if (buffer[strlen(buffer) - 1] == '\n') {
				buffer[strlen(buffer) - 1] = 0; 
			}

			//if exit encountered
			if ((_stricmp(buffer, "exit\0") == 0))
			{
				//set end flag to true
				end = 1;
			}
			printf(">");
		}

		memset(buffer, 0, buffsize);// initialize line buffer to nothing
		//Receive one line at a time in a fixed-size buffer
		nRead = recv(skt, buffer, buffsize, flags);
		if (nRead > 0)
		{
			printf("\n<");
			// Process line(s) in received buffer contents    . . . . . . 
			buffer[nRead] = '\0';  // stuff NULL into position following  
								   // the actual incoming buffer content
								   // print out the buffer
		
			//if there were multiple lines sent, put the < character before each of them
			for (int k = 0; k < (nRead-1); k++)
			{
				if (buffer[k] == '\n')
				{
					printf("\n<");
				}
				else
					printf("%c", buffer[k]);
			}
			printf("\n");
		
			// remove \n from the end of the line before exit condition check
			if (buffer[strlen(buffer) - 1] == '\n') {
				buffer[strlen(buffer) - 1] = 0; 
			}

			//if exit is encountered from other user
			if (_stricmp(buffer, "exit\0") == 0)
			{
				//set end flag to true
				end = 1;
			}
			printf(">");
		}

		memset(buffer, 0, buffsize); // initialize line buffer to nothing
		Sleep(500); // Be sure to force a timeslice if polling
					// to break up a CPU-bound polling loop
	}
	return;
}