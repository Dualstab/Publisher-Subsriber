#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <WinSock2.h>
#include "ConnectionList.h"
#define _CRT_SECURE_NO_WARNINGS
#define table_size 10


unsigned int HashFunction(const char * topic) {

	int length = strlen(topic);
	unsigned int hash_value = 0;
	for (int i = 0; i < length; i++) {

		hash_value += topic[i];
		hash_value = (hash_value *topic[i]) % table_size;
	}

	return hash_value;

}

bool AddToTable(subscribers **subTable, subscribers *s) {
	if (s == NULL) return false;
	int index = HashFunction(s->topic);

	s->next = subTable[index];
	subTable[index] = s;

	return true;
}

subscribers * CreateSubscriber(const char *topic) {

	subscribers * novi = (subscribers *)malloc(sizeof(subscribers));
	if (novi == NULL) return NULL;

	strcpy_s(novi->topic, topic);
	novi->acceptedSocketsForTopic = NULL;
	return novi;
}



void DeleteSubscriberFromListOfSubscribers(subscribers**subTable, SOCKET socket) {
	for (int i = 0; i < table_size; i++) {
		if (subTable[i] == NULL) continue;
		else {
			subscribers *temp = subTable[i];
			while (temp != NULL)
			{
				if (Remove(&temp->acceptedSocketsForTopic, socket))
				{
					printf("Nasao ga i brisem iz liste...\n");
				}
				temp = temp->next;
			}
		}
	}


}


void initTable(subscribers**subTable) {
	for (int i = 0; i < table_size; i++) {
		subTable[i] = NULL;

	}
}

subscribers * FindSubscriberInTable(subscribers**subTable, const char * topic) {
	int index = HashFunction(topic);
	subscribers *temp = subTable[index];
	while (temp != NULL && strcmp(temp->topic, topic) != 0) {
		temp = temp->next;
	}
	return temp;
}

bool DeleteFromTable(subscribers**subTable, char *topic) {
	int index = HashFunction(topic);
	subscribers *temp = subTable[index];
	subscribers *prev = NULL;
	while (temp != NULL && strcmp(temp->topic, topic) != 0) {
		prev = temp;
		temp = temp->next;
	}
	if (temp == NULL) {
		return false;
	}
	if (prev == NULL) {
	
		subTable[index] = temp->next;
	}
	else {
		prev->next = temp->next;
	}

	deleteList(&temp->acceptedSocketsForTopic);
	free(temp);
	temp = NULL;
	return true;
}

void DeleteAllTable(subscribers**subTable) {
	for (int i = 0; i < table_size; i++) {
		if (subTable[i] == NULL) continue;
		else {
			subscribers *temp = subTable[i];
			while (subTable[i] != NULL) {
				DeleteFromTable(subTable, subTable[i]->topic);
			}
		}
	}
}