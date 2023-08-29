#include "Subscriber.h"

int __cdecl main(int argc, char **argv)
{
	int iResult;
	char recvbuf[DEFAULT_BUFLEN];

	if (InitializeWindowsSockets() == false)
	{
		return 1;
	}

	if (Connect()) 
		return 1;

	const char *topic;
	const char* topics[]
		= { "Tech", "Cars", "Aliens", "Etherium" };

	for (int i = 0; i < sizeof(topics) / 8; i++)
		printf("%d. %s \n", i + 1, topics[i]);

	int wasNum = 0;
	int option = -1;
	bool cond = false;

	do {
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
	
	InitializeCriticalSection(&CS_IO);
	HANDLE SubT = CreateThread(NULL, 0, &SubscribeThread, &connectSocket, 0, &SubscriberThreadId);
	FULLMESSAGE *package;
	exit_con = CreateSemaphore(0, 0, 1, NULL);
   
	do{
		iResult = recv(connectSocket, recvbuf, (int)(sizeof(FULLMESSAGE)), 0);
		if (iResult > 0)
		{
			package = (FULLMESSAGE*)recvbuf;
			EnterCriticalSection(&CS_IO);
			printf("Topic: %s\n", package->topic);
			printf("Message: %s \n", package->text);
			LeaveCriticalSection(&CS_IO);
		}
		else if (iResult == 0)
		{
			EnterCriticalSection(&CS_IO);
			printf("Connection closed.\n");
			LeaveCriticalSection(&CS_IO);
			closesocket(connectSocket);
			ReleaseSemaphore(exit_con, 1, NULL);
			break;
		}
		else
		{
			EnterCriticalSection(&CS_IO);
			printf("rcv failed connection error");
			LeaveCriticalSection(&CS_IO);
			closesocket(connectSocket);
			ReleaseSemaphore(exit_con, 1, NULL);
			break;
		}
	} while (WaitForSingleObject(exit_con, 200) == WAIT_TIMEOUT);

	if (SubT) {
		WaitForSingleObject(SubT, INFINITE);
	}
	
	if (SubT)
	{
		CloseHandle(SubT);
	}
	if (exit_con)
	{
		CloseHandle(exit_con);
	}

	DeleteCriticalSection(&CS_IO);
	closesocket(connectSocket);
	WSACleanup();
	return 0;
}
