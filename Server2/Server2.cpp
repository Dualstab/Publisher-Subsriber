#include <conio.h>
#include "Server2.h"

int  main(void)
{
	SOCKET listenSocketPublisher = INVALID_SOCKET;

	int iResultPublisher;

	if (InitializeWindowsSockets() == false)
	{
		return 1;
	}
    //connect to main server
	if (Connect())
	{
		return 1;
	}

	listenSocketPublisher = InitializeListenSocket(PUBLISHER_PORT);
	if (listenSocketPublisher == SOCKET_ERROR || listenSocketPublisher == INVALID_SOCKET)
		return 1;


	// Set listenSocket in listening mode
	iResultPublisher = listen(listenSocketPublisher, SOMAXCONN);
	

	if (iResultPublisher == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocketPublisher);
		WSACleanup();
		return 1;
	}

	printf("Proxy initialized, waiting for publishers.\n");

	InitializeCriticalSection(&criticalSectionForPublisher);

	exit_con = CreateSemaphore(0, 0, 2, NULL);
	publisherSemafor = CreateSemaphore(0, 0, 1, NULL);
	publisherDoneSemafor = CreateSemaphore(0, 0, 1, NULL);


	t1 = CreateThread(NULL, 0, &FunkcijaThread1, &listenSocketPublisher, 0, &thread1ID);
	t2 = CreateThread(NULL, 0, &FunkcijaThread2, NULL, 0, &thread2ID);



	if (!t1 || !t2)
	{
		ReleaseSemaphore(exit_con, 2, NULL);
	}


	while (1) 
	{

		if (_kbhit()) {
			char c = _getch();
			if (c == 'q') {
				ReleaseSemaphore(exit_con, 2, NULL);
				break;
			}
		}
	}

	if (t1) {
		WaitForSingleObject(t1, INFINITE);
	}
	if (t2) {
		WaitForSingleObject(t2, INFINITE);
	}


	if (t1) {
		CloseHandle(t1);
	}
	if (t2) {
		CloseHandle(t2);
	}
	if (exit_con) {
		CloseHandle(exit_con);
	}
	if (publisherSemafor) {
		CloseHandle(publisherSemafor);
	}
	if (publisherDoneSemafor) {
		CloseHandle(publisherDoneSemafor);
	}


	DeleteCriticalSection(&criticalSectionForPublisher);
	CloseAllSocketsForList(publisherSockets);
	deleteList(&publisherSockets);
	closesocket(listenSocketPublisher);
	listenSocketPublisher = INVALID_SOCKET;
	WSACleanup();

	return 0;
}