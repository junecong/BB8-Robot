#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "../Vision/motionTrack.h"
#include "../Globals/externals.h"

#define PORT 51717

// current ip of Maxwell Board, DO NOT CHANGE
static char *ip = "192.168.42.1";
BoundedBuffer bBuffer(2);

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

// convert int to string up to nine digits
// Does not accomodate for leading zeros, or numbers greater than 10 digits
void itoa(int num, char *str) {
    int copy = num;
    int numDigits = 0;
    int negative = 0;
    int base = 1;

    if (num < 0) {
        negative = 1;
    }

    while (copy) {
        base *= 10;
        copy = copy / 10;
        numDigits++;
    }

    base /= 10;

    memset(str, 0, strlen(str));
    copy = num;
    if (numDigits > 9) {
        printf("%s\n", "Floating point exception, defaulting to 0");
        strcat(str, "0");
        return;
    }

    if (num == 0) {
        strcat(str, "0");
        return;
    }

    if (negative) {
        copy *= -1;
        strcat(str, "-");
    }

    int index = negative;
    int i;
    for (i = 0; i < numDigits; i++) {
        char charToAppend = copy/base + '0';
        str[index++] = charToAppend;
        copy %= base;
        base /= 10;
    }

    str[index] = '\0';
}

// removes newline from string
void choppy( char *s ) {
    s[strcspn(s, "\n")] = '\0';
}

// packs Message as 0/*sizeOfPacket*//*[drive/turn]*//*[distance/angle]*//*[percentSpeed/DefaultSpeed]*/1
void packMessage(char *data, char *dist_angle, char *percentSpeed, char *str) {
    choppy(data);
    choppy(dist_angle);
    choppy(percentSpeed);
    int bodySize = strlen(data) + strlen(dist_angle) + strlen(percentSpeed);
    char bodySize_s[20];
    itoa(bodySize, bodySize_s);
    memset(str, 0, strlen(str));
    strcat(str, "0/*");
    strcat(str, bodySize_s);
    strcat(str, "*//*");
    strcat(str, data);
    strcat(str, "*//*");
    strcat(str, dist_angle);
    strcat(str, "*//*");
    strcat(str, percentSpeed);
    strcat(str, "*/1");
}

// main function to send messages to Maxwell board
void setUpSocket(char *argv1, char *argv2) {
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];

    char *defaultIp;
    int defaultPort = PORT;

    if (localhostMode) {
        printf("%d\n", "Using localhost and default port: ", PORT);
        defaultIp = "localhost";
    } else {
        printf("%s %s %s %d\n", "Using default ip: ", ip, " and default port: ", PORT);
        defaultIp = ip;
    }

    // if (argv1 == NULL) {
    //     printf("%s %s %s %d\n", "Using default ip: ", ip, " and default port: ", PORT);
    //     defaultIp = ip;
    //     defaultPort = PORT;
    // } else {
    //     defaultIp = "localhost";
    //     if (argv2 == NULL) {
    //         printf("Using default port: 51717...\n");
    //         defaultPort = PORT;
    //     }
    // }

    //Set up socket
    portno = defaultPort;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        error("ERROR opening socket");
    }

    server = gethostbyname(defaultIp);

    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR connecting");
    }

    while (1) {
        char data[10];
        memset(data, 0, strlen(data));
        char dist_angle[10];
        memset(dist_angle, 0, strlen(dist_angle));
        char percentSpeed[10];
        memset(percentSpeed, 0, strlen(percentSpeed));;

        if (sendMode) {
            memset(buffer, 0, strlen(buffer));
            cout << "input drive command: ";
            cin  >> buffer;
            memcpy(data, buffer, strlen(buffer));
            memset(buffer, 0, strlen(buffer));
            cout << "input dist/degree command: ";
            cin  >> buffer;
            memcpy(dist_angle, buffer, strlen(buffer));
            memset(buffer, 0, strlen(buffer));
            cout << "input Speed percent command: ";
            cin  >> buffer;
            memcpy(percentSpeed, buffer, strlen(buffer));

        } else {
            // fetch from threaded message buffer 
            vector<string> message = bBuffer.fetch();

            // create packet 
            message[0].copy(data, message[0].size());
            message[1].copy(dist_angle, message[1].size());
            message[2].copy(percentSpeed, message[2].size());
        }
        char packet[256];
        memset(packet, 0, strlen(packet));
        packMessage(data, dist_angle, percentSpeed, packet);

        // send data packet
        n = write(sockfd, packet, strlen(packet) + 1);

        if (n < 0) {
            error("ERROR writing to socket");
        }

    }
    close(sockfd);
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        for (int i = 0; i < argc; i++) {
            if (strcmp(argv[i], "localhost") == 0) {
                localhostMode = true;
            }
            if (strcmp(argv[i], "-f") == 0) {
                debugMode = true;
            }
            if (strcmp(argv[i], "-s") == 0) {
                sendMode = true;
            }
        }
    }

    // run message send thread
    thread t1(setUpSocket, argv[1], argv[2]);

    if (!sendMode) {
        analyzeVideo();
    }
    t1.join();

    return 0;
}
