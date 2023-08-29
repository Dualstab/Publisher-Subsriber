#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define DEFAULT_BUFLEN 524
#define PUBLISHER_PORT "27016"
#include "../Common/Structs.h"
#include "../Server/ConnectionList.h"
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include <stdio.h>

CRITICAL_SECTION  criticalSectionForPublisher;
HANDLE exit_con, publisherSemafor, publisherDoneSemafor;
connectedUsers * publisherSockets = NULL; // lista konektovanih publishera
SOCKET connectSocket = INVALID_SOCKET; // konekcija ka main serveru


HANDLE t1, t2;
DWORD thread1ID, thread2ID;


int Connect() {
	// create a socket
	connectSocket = socket(AF_INET,
		SOCK_STREAM,
		IPPROTO_TCP);

	if (connectSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// create and initialize address structure
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddress.sin_port = htons(27011);
	// connect to server specified in serverAddress and socket connectSocket
	if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		printf("Unable to connect to server.\n");
		closesocket(connectSocket);
		WSACleanup();
	}

	return 0;

}

bool InitializeWindowsSockets()
{
	WSADATA wsaData;
	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return false;
	}
	return true;
}




SOCKET InitializeListenSocket(const char* port) {

	SOCKET listenSocket = INVALID_SOCKET;
	// Prepare address information structures
	addrinfo *resultingAddress = NULL;
	addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;       // IPv4 address
	hints.ai_socktype = SOCK_STREAM; // Provide reliable data streaming
	hints.ai_protocol = IPPROTO_TCP; // Use TCP protocol
	hints.ai_flags = AI_PASSIVE;     // 

	// Resolve the server address and port
	int iResult = getaddrinfo(NULL, port, &hints, &resultingAddress);
	if (iResult != 0)
	{
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}


	// Create a SOCKET for connecting to server
    //		IPv4 address famly|stream socket | TCP protocol
	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (listenSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);

		WSACleanup();
		return INVALID_SOCKET;
	}

	// Setup the TCP listening socket - bind port number and local address 
	// to socket
	iResult = bind(listenSocket, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		WSACleanup();
		return INVALID_SOCKET;
	}

	// Since we don't need resultingAddress any more, free it
	freeaddrinfo(resultingAddress);
	//stavi u neblokirjauci rezim
	unsigned long mode = 1;
	iResult = ioctlsocket(listenSocket, FIONBIO, &mode);
	if (iResult != NO_ERROR) {
		printf("ioctlsocket failed with error: %ld\n", iResult);
		return INVALID_SOCKET;
	}
	return listenSocket;
}


DWORD WINAPI FunkcijaThread1(LPVOID param) {

	SOCKET listenSocketPublisher = *(SOCKET *)param;
	SOCKET acceptedSocketPublisher = INVALID_SOCKET; 
	unsigned long mode = 1;

	if (InitializeWindowsSockets() == false)
	{
		return 1;
	}

	fd_set readfds;
	timeval timeVal;
	timeVal.tv_sec = 1;
	timeVal.tv_usec = 0;

	while (WaitForSingleObject(exit_con, 500) == WAIT_TIMEOUT) {

		FD_ZERO(&readfds);
		FD_SET(listenSocketPublisher, &readfds);
		int selectResult = select(0, &readfds, NULL, NULL, &timeVal);
		if (selectResult == SOCKET_ERROR) {

			printf("Publisher listen socket failed with error: %d\n", WSAGetLastError());
			closesocket(listenSocketPublisher);
			WSACleanup();
			return 1;
		}
		else if (selectResult == 0)
		{
			continue;
		}
		else
		{
			acceptedSocketPublisher = INVALID_SOCKET;
			sockaddr_in clientAddr;
			int clinetAddrSize = (sizeof(struct sockaddr_in));
			acceptedSocketPublisher = accept(listenSocketPublisher, (struct sockaddr*)&clientAddr, &clinetAddrSize);
			if (acceptedSocketPublisher == INVALID_SOCKET) 
			{
				printf("Accept Failed.\n");
			}
			else 
			{
				ioctlsocket(acceptedSocketPublisher, FIONBIO, &mode);
				EnterCriticalSection(&criticalSectionForPublisher);
				Add(&publisherSockets, acceptedSocketPublisher);
				LeaveCriticalSection(&criticalSectionForPublisher);
				ReleaseSemaphore(publisherSemafor, 1, NULL);
				printf("New Publisher request accpeted from address: %s : %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

			}
		}

	}

	ReleaseSemaphore(publisherDoneSemafor, 1, NULL);
	closesocket(listenSocketPublisher);
	WSACleanup();
	return 0;
}



DWORD WINAPI FunkcijaThread2(LPVOID param) {

	int iResultPublisher;

	char recvbuf[DEFAULT_BUFLEN];

	fd_set readfds;
	timeval timeVal;
	timeVal.tv_sec = 1;
	timeVal.tv_usec = 0;
	FULLMESSAGE *vest;
	connectedUsers * current = NULL;
	bool finish = false;

	if (InitializeWindowsSockets() == false)
	{
		return 1;
	}

	HANDLE forward_msg_flags[2] = { publisherSemafor, publisherDoneSemafor };

	while (WaitForSingleObject(exit_con, 500) == WAIT_TIMEOUT)
	{

		current = publisherSockets;
		while (current == NULL)
		{
			if (WaitForMultipleObjects(2, forward_msg_flags, FALSE, INFINITE) == WAIT_OBJECT_0 + 1)
			{
				finish = true;
				break;
			}
			current = publisherSockets;
		}

		if (finish) break;

		FD_ZERO(&readfds);

		EnterCriticalSection(&criticalSectionForPublisher);

		while (current != NULL)
		{
			FD_SET(current->acceptedSocket, &readfds);
			current = current->next;
		}
		LeaveCriticalSection(&criticalSectionForPublisher);


		int selectResult = select(0, &readfds, NULL, NULL, &timeVal);

		if (selectResult == SOCKET_ERROR)
		{

			printf("Publisher accpted socket list is probably empty: %d\n", WSAGetLastError());

		}
		else if (selectResult == 0)
		{
			continue;
		}
		else
		{
			EnterCriticalSection(&criticalSectionForPublisher);
			current = publisherSockets;
			while (current != NULL)
			{   
				if (FD_ISSET(current->acceptedSocket, &readfds)) {
					
					iResultPublisher = recv(current->acceptedSocket, recvbuf, (int)(sizeof(FULLMESSAGE)), 0);
					if (iResultPublisher > 0)
					{
						vest = (FULLMESSAGE*)recvbuf;
						
						FULLMESSAGE forsend;
						strcpy_s(forsend.topic, (char*)vest->topic);
						strcpy_s(forsend.text, (char*)vest->text);

						printf("Topic: %s\n", vest->topic);
						printf("Poruka: %s \n\n", vest->text);

						int iResult = send(connectSocket, (char *)&forsend, (int)sizeof(FULLMESSAGE), 0);
						
						current = current->next; 
					}
					else if (iResultPublisher == 0)
					{
						printf("Publisher disconected.\n");
						closesocket(current->acceptedSocket);

						connectedUsers * forDelete = current;
						current = current->next; 
						Remove(&publisherSockets, forDelete->acceptedSocket);

					}
					else
					{
						printf("Publisher error: %d\n", WSAGetLastError());
						closesocket(current->acceptedSocket);

						connectedUsers * forDelete = current;
						current = current->next;
						Remove(&publisherSockets, forDelete->acceptedSocket);

					}

				}
				else current = current->next;

			}
			LeaveCriticalSection(&criticalSectionForPublisher);
		}

	}
	WSACleanup();
	return 0;
}




