#pragma once
#include <WinSock2.h>
#define max_topic 15


typedef struct connectedUsers {
	SOCKET acceptedSocket;
	struct connectedUsers *next;
}CONNECTEDUSERS;


//Queue
typedef struct fullMsg {
	char topic[max_topic];
	char text[100];
} FULLMESSAGE;

typedef struct queue {
	struct node *front;
	struct node *back;
} QUEUE;

typedef struct node {
	struct fullMsg data;
	struct node *next;
} NODE;



typedef struct subscribers {
	char topic[max_topic];
	connectedUsers * acceptedSocketsForTopic;
	struct subscribers *next;
}subscribers;
