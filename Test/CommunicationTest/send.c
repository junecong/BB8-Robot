#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define PORT 51717

// change ip to ip of device
// current ip after wifi script, DO NOT CHANGE
// static char *ip = "192.168.42.1";
static char *ip = "192.168.42.1";

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

// Packs Message as 0/*sizeOfPacket*//*idOfSensor*//*data*/1
void packMessage(int id, char *data, char *str) {
    // BUG: for some reason, putting itoa(data)
    // before itoa(id) causes the data to be erased
    // if not inputing from stdin
    char id_s[15];
    itoa(id, id_s);
    choppy(data);
    int bodySize = strlen(data) + strlen(id_s);
    char bodySize_s[20];
    itoa(bodySize, bodySize_s);

    memset(str, 0, strlen(str));
    strcat(str, "0");
    strcat(str, "/*");
    strcat(str, bodySize_s);
    strcat(str, "*/");
    strcat(str, "/*");
    strcat(str, id_s);
    strcat(str, "*/");
    strcat(str, "/*");
    printf("data: %s\n", data);
    strcat(str, data);
    strcat(str, "*/");
    strcat(str, "1");
    printf("HERE:  %s\n", str);
}

int main(int argc, char *argv[]) {
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];

    char *defaultIp;
    int defaultPort;

    if (argv[1] == NULL) {
        printf("%s %s %s %d\n", "Using default ip: ", ip, " and default port: ", PORT);
        defaultIp = ip;
        defaultPort = PORT;
    } else {
        defaultIp = argv[1];
        if (argv[2] == NULL) {
            printf("Using default port: 51717...\n");
            defaultPort = PORT;
        } else {
            defaultPort = atoi(argv[2]);
        }
    }

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

    // test id for sensor
    int testID = 0;

    while (1) {
        // Simulate number of sensors
        if (testID > 10) {
            testID = 0;
        } 

        // for testing, input data
        printf("Please enter the message: ");
        memset(buffer, 0, 256);
        fgets(buffer, 255, stdin);

        int i = atoi(buffer);
        // create data packet
        char temp[256];
        packMessage(testID, buffer, temp);
        int sizeofPacket = strlen(temp) + 1;
        char packet[sizeofPacket];
        memset(packet, 0, sizeofPacket);
        memcpy(packet, temp, sizeofPacket);

        // send data packet
        n = write(sockfd, packet, strlen(packet) + 1);

        if (n < 0) {
            error("ERROR writing to socket");
        }
        memset(buffer, 0, 256);
        n = read(sockfd,buffer,255);

        if (n < 0) {
            error("ERROR reading from socket");
        }

        if (strcmp(buffer, "done") == 0) {
            break;
        }
        // increment sensorId to test multiple sensors
        testID+=1;
    }
    close(sockfd);
    return 0;
}