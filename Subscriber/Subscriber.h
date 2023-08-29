#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "../Common/Structs.h"
#pragma comment(lib, "ws2_32.lib")

#define WIN32_LEAN_AND_MEAN
#define DEFAULT_BUFLEN 524
#define DEFAULT_PORT 27017

DWORD WINAPI SubscribeThread(LPVOID param);
void Subscribe(void*topic);
int Connect();
bool InitializeWindowsSockets();

HANDLE exit_con;
DWORD SubscriberThreadId;
CRITICAL_SECTION CS_IO;
SOCKET connectSocket = INVALID_SOCKET;

int Connect() {

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
	serverAddress.sin_port = htons(DEFAULT_PORT);
	// connect to server specified in serverAddress and socket connectSocket
	if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		printf("Unable to connect to server.\n");
		closesocket(connectSocket);
		WSACleanup();
	}
	return 0;
}
//send topic to server
void Subscribe(void*topic) {

	char* topicCoice = (char*)topic;

	int iResult_s = send(connectSocket, topicCoice, strlen(topicCoice), 0);

	if (iResult_s == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
	}
	printf("Msg sent.\n");

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

DWORD WINAPI SubscribeThread(LPVOID param)
{
	SOCKET connectSocket = *(SOCKET*)param;

	while (WaitForSingleObject(exit_con, 400) == WAIT_TIMEOUT)
	{
		if (_kbhit()) 
		{
			EnterCriticalSection(&CS_IO);
			char input = _getch();
			LeaveCriticalSection(&CS_IO);
			if (input == 'a')
			{
				EnterCriticalSection(&CS_IO);
				const char *topic;
				const char* topics[]
					= { "Tech", "Cars", "Aliens", "Etherium" };

				for (int i = 0; i < sizeof(topics) / 8; i++)
					printf("%d. %s \n", i + 1, topics[i]);

				int wasNum = 0;
				int option = -1;
				bool cond = false;

				do 
				{
					option = -1;
					printf("Unesite broj teme: ");
					wasNum = scanf_s("%d", &option);
					while (wasNum != 1)
					{
						printf("Unesite broj. \n");
						scanf_s("%*[^\n]");
						wasNum = scanf_s("%d", &option);
					}
					if (option > sizeof(topics) / 8 || option <= 0)
					{
						printf("Broj je van opsega\n");
						cond = false;

					}
					else
					{
						cond = true;
					}
					fflush(stdin);

				} while (cond == false);

				topic = topics[option - 1];
				printf("Izabrali ste: %s\n", topic);

				Subscribe((void*)topic);
				LeaveCriticalSection(&CS_IO);
			}
			else if (input == 'x') {
				ReleaseSemaphore(exit_con, 1, NULL);
				break;
			}

		}
	}

	closesocket(connectSocket);
	WSACleanup();
	return 0;
}


