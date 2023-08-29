#include "Publisher.h"

int __cdecl main(int argc, char **argv)
{
	int iResult;
	char *messageToSend = (char*)malloc(sizeof(char) * DEFAULT_BUFLEN);

	if (InitializeWindowsSockets() == false)
		return 1;

	if (Connect())
		return 1;

	const char *topic;
	const char *message;
	const char* topics[]
		= { "Tech", "Cars", "Aliens", "Etherium"};

	do {

		int wasNum = 0;
		int option = -1;
		bool cond = false;

		for (int i = 0; i < sizeof(topics)/8; i++)
			printf("%d. %s \n", i+1, topics[i]);
		
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

		topic = topics[option-1];
		printf("Izabrali ste: %s\n", topic);
		pMsg(messageToSend);
		message = messageToSend;

		//TO DO: napraviti exit condition
		
		iResult = SendTopic((void*)topic, (void*)message);
		if (iResult == -1) break;
		printf("Msg sent.\n");

	} while (true);

	free(messageToSend);
	closesocket(connectSocket);
	WSACleanup();

	return 0;
}
