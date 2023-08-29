#pragma once
#include<stdio.h>
#include <stdlib.h>
#include "../Common/Structs.h"


void InitializeQUEUE(QUEUE* q) {
	q->front = NULL;
	q->back = NULL;
}


void Enqueue(QUEUE* q, FULLMESSAGE a) {
	struct node* newOne = (struct node*)malloc(sizeof(struct node));
	newOne->data = a;
	newOne->next = NULL;

	if (q->back != NULL) {
		q->back->next = newOne;
	}

	q->back = newOne;

	if (q->front == NULL) {
		q->front = newOne;
	}
}

bool Dequeue(QUEUE* q, FULLMESSAGE* retVal) {
	if (q->front == NULL) {
		printf("Q je prazan\n");
		return false;
	}

	NODE* temp = q->front;
	*retVal = temp->data;

	q->front = q->front->next;
	if (q->front == NULL) {
		q->back = NULL;
	}

	free(temp);
	return true;
}

void ShowQueue(QUEUE* q) {
	NODE* current = q->front;

	while (current != NULL) {
		printf("%s %s\n", current->data.topic, current->data.text);
		current = current->next;
	}

}

void ClearQueue(QUEUE *q) {
	FULLMESSAGE retVal;
	while (Dequeue(q, &retVal))
	{
		continue;
	}
}