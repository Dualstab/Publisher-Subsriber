#pragma once
#include<stdio.h>
#include<stdlib.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h> 
#include "../Common/Structs.h"


bool Add(connectedUsers **head, SOCKET data) {
	connectedUsers * new_node;
	new_node = (connectedUsers *)malloc(sizeof(connectedUsers));
	if (new_node == NULL) {
		return false;
	}

	new_node->acceptedSocket = data;
	new_node->next = *head;
	*head = new_node;
	return true;
}

bool Remove(connectedUsers **head, SOCKET sock) {
	connectedUsers * current = *head;
	connectedUsers * previous = NULL;

	if (current == NULL) {
		return false;
	}

	while (current->acceptedSocket != sock) {

		if (current->next == NULL) {
			return false;
		}
		else {
			previous = current;
			//move to next link
			current = current->next;
		}
	}

	if (current == *head) {
		//change first to point to next link
		*head = (*head)->next;
	}
	else {
		//bypass the current link
		previous->next = current->next;
	}

	free(current);
	current = NULL;
	return true;
}

void deleteList(connectedUsers **head) {
	connectedUsers *temp = NULL;
	connectedUsers *current = *head;

	while (current != NULL) {
		temp = current;
		current = current->next;
		free(temp);
		temp = NULL;
	}
	*head = current;
}
bool FindInList(connectedUsers **head, SOCKET s) {

	connectedUsers *temp = NULL;
	connectedUsers *current = *head;
	while (current != NULL) {
		if (current->acceptedSocket == s)
			return true;
		current = current->next;
	}

	return false;
}

void CloseAllSocketsForList(connectedUsers * list) {
	int iResult;

	while (list != NULL) {
		iResult = shutdown(list->acceptedSocket, SD_SEND);
		if (iResult == SOCKET_ERROR)
		{
			printf("Error: %d\n", WSAGetLastError());
			closesocket(list->acceptedSocket);
			WSACleanup();
		}
		closesocket(list->acceptedSocket);
		list = list->next;
	}
}
