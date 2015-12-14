/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "../Servo/motorControl.h"

#define PORTNO 51717
#define MAXQUEUESIZE 10

pthread_mutex_t mutex1;
pthread_cond_t dataReady;
pthread_cond_t notFull;

typedef struct node {
    char *message;
    struct node *next;
} node_t;

typedef struct queue {
    int size;
    node_t *head;
    node_t *tail;
} queue_t;


void initQueue(queue_t *queue) {
    queue->size = 0;
}

void enqueue(queue_t *queue, char *msg) {
    // add lock
    pthread_mutex_lock(&mutex1);
    while (queue->size >= MAXQUEUESIZE) {
        pthread_cond_wait(&notFull, &mutex1);
    }
    node_t *node = (node_t *)malloc(sizeof(node_t));
    node->message = msg;
    if (queue->head) {
        queue->tail->next = node;
        queue->tail = node;
    } else {
        queue->head = node;
        queue->tail = node;
    }
    queue->size++;
    node_t *tmp = queue->head;
    while(tmp) {
        printf("%s\n", tmp->message);
        tmp = tmp->next;
    }
    // signal
    pthread_cond_signal(&dataReady);
    // release lock
    pthread_mutex_unlock(&mutex1);
}

node_t *dequeue(queue_t *queue) {
    // accquire lock
    pthread_mutex_lock(&mutex1);

    while (queue->size <= 0) {
        pthread_cond_wait(&dataReady, &mutex1);
    }

    if (queue->head) {
        node_t *temp = queue->head;
        queue->head = temp->next;
        pthread_cond_signal(&notFull);
        pthread_mutex_unlock(&mutex1);
        queue->size--;
        return temp;
    }
    // relase lock
    pthread_cond_signal(&notFull);
    pthread_mutex_unlock(&mutex1);
    return NULL;
}

void decodePacket(char *packet, int len, char size[], char data[], char dist_angle[], char percentSpeed[]);

void *dequeueMessages(void *arg) {
    queue_t *queue = (queue_t *)arg;

    char buffer[256];
    char size[10];
    char data[20];
    char dist_angle[10];
    char percentSpeed[10];

    while (1) {
        memset(size, 0, 10);
        memset(data, 0, 20);
        memset(dist_angle, 0, 10);
        memset(percentSpeed, 0, 10);
        memset(buffer, 0, 256);
        
        node_t *node = dequeue(queue);
        printf("node message: %s\n", node->message);
        memcpy(buffer, node->message, strlen(node->message));
        // free(node);

        decodePacket(buffer, strlen(buffer), size, data, dist_angle, percentSpeed);
        float fdist_angle = 0;
        float fpercentSpeed = 0.7;
        if (strcmp(data, "error") == 0) {
            memcpy(data, "stop", 4);
        } else {
            if (strcmp(dist_angle, "error") != 0) {
                fdist_angle = atof(dist_angle);
            }
            if (strcmp(percentSpeed, "error") != 0) {
                fpercentSpeed = atof(percentSpeed);
            }
        }

        printf("%s %f %f\n", data, fdist_angle, fpercentSpeed);

        if (strcmp(data, "exit") == 0) {
            move("stop", 0, 0);
            break;
        } else {
            // call to drive motors in Servo/motorControl.cpp
            move(data, fdist_angle, fpercentSpeed);
        }
    }
}

int move(char data[], int speedVal);

void error(const char *msg) {
    perror(msg);
    exit(1);
}

// puts packet string into data array
void getString (char *packet, char data[], int *index) {
    int i = 0;
    while (packet[*index] != '*') {
        data[i] = packet[(*index)++];
        i++;
    }

    data[i] = '\0';
    *index += 4;

    // check if data exists
    if (data[0] == '\0') {
        memcpy(data, "error", 5);
    }
}

// Decodes received message and puts them into the respective char array
void decodePacket(char *packet, int len, char size[], char data[], char dist_angle[], char percentSpeed[]) {
    // printf("%s\n", packet);
    if (packet[0] == '0' && packet[len-1] == '1') {
        int index = 3;
        getString(packet, size, &index);
        getString(packet, data, &index);
        getString(packet, dist_angle, &index);
        getString(packet, percentSpeed, &index);
    } else {
        printf("%s\n", "segmented packet: Incorrect Start and End");
    }
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    int defaultPort;
    if (argc < 2) {
        printf("Using default port: 51717...\n");
       defaultPort = PORTNO;
    } else {
       defaultPort = atoi(argv[1]);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) 
       error("ERROR opening socket");

    memset((char *) &serv_addr, 0, sizeof(serv_addr));

    portno = defaultPort;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR on binding");
    }

    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

    if (newsockfd < 0) {
        error("ERROR on accept");
    }

    // start threading
    if (pthread_mutex_init(&mutex1, NULL) != 0) {
        printf("\n mutex init failed\n");
        return 1;
    }

    if (pthread_cond_init(&dataReady, NULL) != 0) {
        printf("\n mutex init failed\n");
        return 1;
    }

    if (pthread_cond_init(&notFull, NULL) != 0) {
        printf("\n mutex init failed\n");
        return 1;
    }
    pthread_t tid[2];
    queue_t *queue = (queue_t *)malloc(sizeof(queue_t));
    initQueue(queue);
    pthread_create(&(tid[1]), NULL, dequeueMessages, queue);

    while (1) {

        memset(buffer, 0, 256);

        // read data
        // n = read(newsockfd,buffer,255);
        n = read(newsockfd, buffer, sizeof(buffer));

        enqueue(queue, buffer);
        usleep(1000);

        if (n < 0) {
            error("ERROR reading from socket");
        }

        printf("Here is the message: %s\n", buffer);

    }

    close(newsockfd);
    close(sockfd);
    return 0; 
}
