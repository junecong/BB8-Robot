/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define PORTNO 51717

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void decodePacket(char *packet, int size) {
    if (packet[0] == '0' && packet[size-1] == '1') {
        int index = 3;
        char *id;
        while (packet[index] != '*') {
            // strcat(id, packet[index++]);
            printf("%c\n", packet[index++]);
        }
    } else {
        printf("%s\n", "segmented packet: Incorrect Start and End");
    }
}

// void main() {
//     char *test = "0/*8*//*2*//*1231413*/1";
//     int size = strlen(test);
//     decodePacket(test, size);
// }

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

    while (1) {
        memset(buffer, 0, 256);
        // read data
        // n = read(newsockfd,buffer,255);
        n = read(newsockfd, buffer, sizeof(buffer));

        if (n < 0) {
            error("ERROR reading from socket");
        }

        printf("Here is the message: %s\n", buffer);
        //decode buffer
        decodePacket(buffer, strlen(buffer));

        n = write(newsockfd, "I got your message", 18);

        if (n < 0) {
            error("ERROR writing to socket");
        }
    }

    close(newsockfd);
    close(sockfd);
    return 0; 
}