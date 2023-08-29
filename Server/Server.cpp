#include <conio.h>
#include "Server.h"


int  main(void)
{
	SOCKET listenSocketProxy = INVALID_SOCKET;
	SOCKET listenSocketSubscriber = INVALID_SOCKET;
	int iResultProxy, iResultSubscriber;

	if (InitializeWindowsSockets() == false)
	{
		// we won't log anything since it will be logged
		// by InitializeWindowsSockets() function
		return 1;
	}

	listenSocketProxy = InitializeListenSocket(SERVER_PORT);
	if (listenSocketProxy == SOCKET_ERROR || listenSocketProxy == INVALID_SOCKET)
		return 1;
	listenSocketSubscriber = InitializeListenSocket(SUBSCRIBER_PORT);
	if (listenSocketSubscriber == SOCKET_ERROR || listenSocketSubscriber == INVALID_SOCKET)
		return 1;

	iResultProxy = listen(listenSocketProxy, SOMAXCONN); //specify the maximum number of connection requests queued for any listening socket
	iResultSubscriber = listen(listenSocketSubscriber, SOMAXCONN);

	if (iResultProxy == SOCKET_ERROR || iResultSubscriber == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocketProxy);
		closesocket(listenSocketSubscriber);
		WSACleanup();
		return 1;
	}

	printf("Server initialized, waiting for clients.\n");


	//inicijalizuj prazan q,tabelu, i kritizne sekcije
	InitializeQUEUE(&queue); //
	initTable(subTable); // 

	InitializeCriticalSection(&criticalSectionForQueue);
	InitializeCriticalSection(&criticalSectionForProxy);
	InitializeCriticalSection(&criticalSectionForSubscribers);

	exit_con = CreateSemaphore(0, 0, 5, NULL);
	Full = CreateSemaphore(0, 0, MAX_QUEUE_SIZE, NULL);
	Empty = CreateSemaphore(0, MAX_QUEUE_SIZE, MAX_QUEUE_SIZE, NULL);
	proxySemafor = CreateSemaphore(0, 0, 1, NULL);
	proxyDoneSemafor = CreateSemaphore(0, 0, 1, NULL);


	t1 = CreateThread(NULL, 0, &FunkcijaThread1, &listenSocketProxy, 0, &thread1ID);
	t2 = CreateThread(NULL, 0, &FunkcijaThread2, &listenSocketSubscriber, 0, &thread2ID);
	t3 = CreateThread(NULL, 0, &FunkcijaThread3, NULL, 0, &thread3ID);

	//pool za slanje sa q
	t4 = CreateThread(NULL, 0, &FunkcijaThread4, (LPVOID)0, 0, &thread4ID);
	t5 = CreateThread(NULL, 0, &FunkcijaThread4, (LPVOID)1, 0, &thread5ID);

	if (!t1 || !t2 || !t3 || !t4 || !t5) {
		ReleaseSemaphore(exit_con, 5, NULL);
	}
	//exit server
	while (1) {

		if (_kbhit()) {
			char c = _getch();
			if (c == 'q') {
				ReleaseSemaphore(exit_con, 5, NULL);
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
	if (t3) {
		WaitForSingleObject(t3, INFINITE);
	}
	if (t4) {
		WaitForSingleObject(t4, INFINITE);
	}
	if (t5) {
		WaitForSingleObject(t5, INFINITE);
	}

	if (t1) {
		CloseHandle(t1);
	}
	if (t2) {
		CloseHandle(t2);
	}
	if (t3) {
		CloseHandle(t3);
	}
	if (t4) {
		CloseHandle(t4);
	}
	if (t5) {
		CloseHandle(t5);
	}
	if (exit_con) {
		CloseHandle(exit_con);
	}
	if (Empty) {
		CloseHandle(Empty);
	}
	if (Full) {
		CloseHandle(Full);
	}
	if (proxySemafor) {
		CloseHandle(proxySemafor);
	}
	if (proxyDoneSemafor) {
		CloseHandle(proxyDoneSemafor);
	}


	DeleteCriticalSection(&criticalSectionForQueue);
	DeleteCriticalSection(&criticalSectionForProxy);
	DeleteCriticalSection(&criticalSectionForSubscribers);
	ClearQueue(&queue);
	CloseAllSocketsForList(proxySocket);
	CloseAllSocketsForList(subscriberSockets);
	DeleteAllTable(subTable); 
	deleteList(&proxySocket);
	deleteList(&subscriberSockets);
	closesocket(listenSocketProxy);
	closesocket(listenSocketSubscriber);
	listenSocketProxy = INVALID_SOCKET;
	listenSocketSubscriber = INVALID_SOCKET;
	WSACleanup();

	return 0;
}