#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "SubscriberDB.h"
#include "Queue.h"
#define DEFAULT_BUFLEN 524
#define MAX_QUEUE_SIZE 100
#define SERVER_PORT "27011"
#define SUBSCRIBER_PORT "27017"
#pragma comment(lib, "ws2_32.lib") 

QUEUE queue;
CRITICAL_SECTION criticalSectionForQueue, criticalSectionForProxy, criticalSectionForSubscribers;
HANDLE exit_con, Full, Empty, proxySemafor, proxyDoneSemafor;
connectedUsers * proxySocket = NULL; 
connectedUsers * subscriberSockets = NULL;
subscribers *subTable[table_size];



HANDLE t1, t2, t3, t4, t5;
DWORD thread1ID, thread2ID, thread3ID, thread4ID, thread5ID;


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

	SOCKET listenSocketProxy = *(SOCKET *)param; 
	SOCKET acceptedSocketProxy = INVALID_SOCKET; 
	unsigned long mode = 1;

	if (InitializeWindowsSockets() == false)
	{
		// we won't log anything since it will be logged
		// by InitializeWindowsSockets() function
		return 1;
	}

	fd_set readfds;
	timeval timeVal;
	timeVal.tv_sec = 1;
	timeVal.tv_usec = 0;

	while (WaitForSingleObject(exit_con, 500) == WAIT_TIMEOUT) {

		FD_ZERO(&readfds);
		FD_SET(listenSocketProxy, &readfds);

		int selectResult = select(0, &readfds, NULL, NULL, &timeVal);
		if (selectResult == SOCKET_ERROR) {

			printf("Publisher listen socket failed with error: %d\n", WSAGetLastError());
			closesocket(listenSocketProxy);
			WSACleanup();
			return 1;
		}
		else if (selectResult == 0) 
		{
			continue;
		}
		else {
			acceptedSocketProxy = INVALID_SOCKET;
			sockaddr_in clientAddr;
			int clinetAddrSize = (sizeof(struct sockaddr_in));
			acceptedSocketProxy = accept(listenSocketProxy, (struct sockaddr*)&clientAddr, &clinetAddrSize);
			if (acceptedSocketProxy == INVALID_SOCKET) {
				
				printf("Accept Failed.\n");
			}
			else {
				ioctlsocket(acceptedSocketProxy, FIONBIO, &mode);
				EnterCriticalSection(&criticalSectionForProxy);
				Add(&proxySocket, acceptedSocketProxy);
				LeaveCriticalSection(&criticalSectionForProxy);
				ReleaseSemaphore(proxySemafor, 1, NULL);
				printf("Publisher proxy connected from address: %s : %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
				//break;
				//gasimo thread cim se konektuje proxy
				
			}
		}

	}

	ReleaseSemaphore(proxyDoneSemafor, 1, NULL);
	closesocket(listenSocketProxy);
	WSACleanup();
	return 0;
}


DWORD WINAPI FunkcijaThread2(LPVOID param) {

	SOCKET listenSocketSubscriber = *(SOCKET*)param;
	SOCKET acceptedSocketSubscriber = INVALID_SOCKET;


	int iResultSubscriber;

	char recvbuf[DEFAULT_BUFLEN];

	fd_set readfds;
	timeval timeVal;
	timeVal.tv_sec = 1;
	timeVal.tv_usec = 0;
	unsigned long mode = 1;



	connectedUsers * current = NULL;

	if (InitializeWindowsSockets() == false)
	{
		// we won't log anything since it will be logged
		// by InitializeWindowsSockets() function
		return 1;
	}

	while (WaitForSingleObject(exit_con, 1000) == WAIT_TIMEOUT) {
		
		current = subscriberSockets;

		FD_ZERO(&readfds);

		FD_SET(listenSocketSubscriber, &readfds);
		
		while (current != NULL)
		{
			FD_SET(current->acceptedSocket, &readfds);
			current = current->next;
		}
		
		int selectResult = select(0, &readfds, NULL, NULL, &timeVal);

		if (selectResult == SOCKET_ERROR) {

			printf("Select failed with error: %d\n", WSAGetLastError());

		}
		else if (selectResult == 0) {

			continue;
		}
		else {
		
			if (FD_ISSET(listenSocketSubscriber, &readfds)) 
			{
				sockaddr_in clientAddr;
				int clinetAddrSize = (sizeof(struct sockaddr_in));

				acceptedSocketSubscriber = accept(listenSocketSubscriber, (struct sockaddr*)&clientAddr, &clinetAddrSize);

				if (acceptedSocketSubscriber == INVALID_SOCKET) 
				{
		        	printf("Accept error.\n");
				}
				else 
				{
					ioctlsocket(acceptedSocketSubscriber, FIONBIO, &mode);
					Add(&subscriberSockets, acceptedSocketSubscriber);
					printf("New subscriber request accpeted on address: %s : %d\n",inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
				}
				
			}
			else 
			{
				current = subscriberSockets;
				while (current != NULL)
				{
					if (FD_ISSET(current->acceptedSocket, &readfds)) {

						iResultSubscriber = recv(current->acceptedSocket, recvbuf, DEFAULT_BUFLEN, 0);
						if (iResultSubscriber > 0)
						{
							recvbuf[iResultSubscriber] = '\0';
							printf("Subscriber sent: %s.\n", recvbuf);

							subscribers * temp = FindSubscriberInTable(subTable, recvbuf);
							if (temp == NULL) {
								subscribers * novi = CreateSubscriber(recvbuf);
								if (novi == NULL)
									printf("Failed to add topic and subscriber.\n");
								else 
								{
									EnterCriticalSection(&criticalSectionForSubscribers);
									if (Add(&novi->acceptedSocketsForTopic, current->acceptedSocket)) 
									{
										printf("Subscriber info added to the topic structure.\n");
										if (AddToTable(subTable, novi)) 
											printf("Added user to the topic list\n");
										else  
											printf("Failed to subscribe\n");
									}
									else printf("Failed to subscribe");
									LeaveCriticalSection(&criticalSectionForSubscribers);
								}
							}
							else 
							{   
								EnterCriticalSection(&criticalSectionForSubscribers);
								if (!FindInList(&temp->acceptedSocketsForTopic, current->acceptedSocket)) 
								{
									if (Add(&temp->acceptedSocketsForTopic, current->acceptedSocket)) 
										printf("Topic exists, adding subscriber info.\n");
									else 
										printf("Failed to subscribe\n");
								}
								else
									printf("User is already subscribed to the topic.\n");
								LeaveCriticalSection(&criticalSectionForSubscribers);
							}
							current = current->next;
						}
						else if (iResultSubscriber == 0)
						{

							printf("Subscriber connection closed.\n");
							closesocket(current->acceptedSocket);

							EnterCriticalSection(&criticalSectionForSubscribers);
							DeleteSubscriberFromListOfSubscribers(subTable, current->acceptedSocket);
							LeaveCriticalSection(&criticalSectionForSubscribers);
							

							connectedUsers * forDelete = current;
							current = current->next;

							Remove(&subscriberSockets, forDelete->acceptedSocket);

						}
						else
						{
							printf("Error: %d\n", WSAGetLastError());
							closesocket(current->acceptedSocket);
							EnterCriticalSection(&criticalSectionForSubscribers);
							DeleteSubscriberFromListOfSubscribers(subTable, current->acceptedSocket);
							LeaveCriticalSection(&criticalSectionForSubscribers);
							
							connectedUsers * forDelete = current;
							current = current->next;
							Remove(&subscriberSockets, forDelete->acceptedSocket);
						}

					}
					else current = current->next;
				}

			}
		}
	}
	closesocket(listenSocketSubscriber);
	WSACleanup();
	return 0;
}


DWORD WINAPI FunkcijaThread3(LPVOID param) {

	int iResultProxy;

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

	HANDLE write_to_q_flags[2] = { proxySemafor, proxyDoneSemafor };

	while (WaitForSingleObject(exit_con, 500) == WAIT_TIMEOUT)
	{
		current = proxySocket;
		while (current == NULL) 
		{
			if (WaitForMultipleObjects(2, write_to_q_flags, FALSE, INFINITE) == WAIT_OBJECT_0 + 1)
			{
				finish = true;
				break;
			}
			current = proxySocket;
		}

		if (finish) break;

		FD_ZERO(&readfds);

		EnterCriticalSection(&criticalSectionForProxy);

		while (current != NULL)
		{
			FD_SET(current->acceptedSocket, &readfds);
			current = current->next;
		}
		LeaveCriticalSection(&criticalSectionForProxy);


		int selectResult = select(0, &readfds, NULL, NULL, &timeVal);

		if (selectResult == SOCKET_ERROR)
		{

			printf("Proxy list is probably empty: %d\n", WSAGetLastError());

		}
		else if (selectResult == 0)
		{
			continue;
		}
		else
		{
			EnterCriticalSection(&criticalSectionForProxy);
			current = proxySocket;
			while (current != NULL)
			{  
				if (FD_ISSET(current->acceptedSocket, &readfds)) {
					
					iResultProxy = recv(current->acceptedSocket, recvbuf, (int)(sizeof(FULLMESSAGE)), 0);
					if (iResultProxy > 0)
					{
						vest = (FULLMESSAGE*)recvbuf;
						WaitForSingleObject(Empty, INFINITE); 
						EnterCriticalSection(&criticalSectionForQueue);

						printf("Topic: %s\n", vest->topic);
						printf("Poruka: %s \n\n", vest->text);			

						Enqueue(&queue, *vest);
						LeaveCriticalSection(&criticalSectionForQueue);
						current = current->next; 
						ReleaseSemaphore(Full, 1, NULL);
					}
					else if (iResultProxy == 0)
					{
						printf("Proxy disconected.\n");
						closesocket(current->acceptedSocket);

						connectedUsers * forDelete = current;
						current = current->next;
						Remove(&proxySocket, forDelete->acceptedSocket);

					}
					else
					{
						printf("Proxy error: %d\n", WSAGetLastError());
						closesocket(current->acceptedSocket);
						connectedUsers * forDelete = current;
						current = current->next;
						Remove(&proxySocket, forDelete->acceptedSocket);
					}

				}
				else current = current->next;
			}
			LeaveCriticalSection(&criticalSectionForProxy);
		}

	}
	WSACleanup();
	return 0;
}

DWORD WINAPI FunkcijaThread4(LPVOID param) {

	int iResult;
	HANDLE read_from_q_flags[2] = { exit_con,Full };
	FULLMESSAGE vest;

	while (WaitForMultipleObjects(2, read_from_q_flags, FALSE, INFINITE) == WAIT_OBJECT_0 + 1) {

		EnterCriticalSection(&criticalSectionForQueue);
		if (Dequeue(&queue, &vest))
		{
			printf("Removed from queue:\n");
			printf("Topic: %s \n", vest.topic);
			printf("Message: %s\n\n", vest.text);
		}
		else
		{
			printf("Dequeue error.\n");		
		}
		LeaveCriticalSection(&criticalSectionForQueue);
		ReleaseSemaphore(Empty, 1, NULL);


		connectedUsers * listOfSubscribersForTopic = NULL;

		EnterCriticalSection(&criticalSectionForSubscribers);
		subscribers * temp = FindSubscriberInTable(subTable, vest.topic);
		LeaveCriticalSection(&criticalSectionForSubscribers);

		if (temp != NULL) {

			EnterCriticalSection(&criticalSectionForSubscribers);

			listOfSubscribersForTopic = temp->acceptedSocketsForTopic;

			while (listOfSubscribersForTopic != NULL) {

				iResult = send(listOfSubscribersForTopic->acceptedSocket, (char *)&vest, (int)(sizeof(FULLMESSAGE)), 0);

				listOfSubscribersForTopic = listOfSubscribersForTopic->next;

			}
			LeaveCriticalSection(&criticalSectionForSubscribers);
		}
	}

	return 0;
}


