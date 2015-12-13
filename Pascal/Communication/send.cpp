#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <thread>
#include "../Vision/motionTrack.h"

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


    // start opencv
    VideoCapture cap;
    deque <Point2f> objectPoints;
    deque <Point2f> destPoints;
    deque <float> objectRadii;
    deque <float> destRadii;

    bool debugMode = false;

    // detect debug mode
    // if (argc > 1) {
    //  char *flag = argv[1];
    //  if (strcmp(flag, "-d") == 0) {
    //      debugMode = true;
    //  }
    // }

    // Set up camera
    if (!cap.open(1)) {
        cout << "Error detecting camera1" << endl;
        if (!cap.open(0)) {
            cout << "Error detecting camera0" << endl;
            return -1;
        }
    }

    Scalar lowerBoundObject = Scalar(0, 0, 0);
    Scalar upperBoundObject = Scalar(120, 255, 255);

    Scalar lowerBoundDest = Scalar(0, 0, 0);
    Scalar upperBoundDest = Scalar(120, 255, 255);

    // for calibrating Object
    userInput(cap, &lowerBoundObject, &upperBoundObject, "Object-HSV.txt");
    // for calibrating Destination
    userInput(cap, &lowerBoundDest, &upperBoundDest, "Destination-HSV.txt");

    namedWindow("drawing", WINDOW_NORMAL);
    resizeWindow("drawing", 600, 600);

    Point2f objectCenter;
    Point2f prev_objectCenter = objectCenter;
    Point2f destCenter;
    Point2f prev_destCenter = destCenter;
    float objectRadius;
    float prev_objectRadius;
    float destRadius;
    float prev_destRadius;

    double max_dist = 0;
    double dist = 0;
    int count = 0;

    while (1) {
        // start opencv
        string direction = "Stationary";
        bool isOffscreen = true;
        Mat frame, mask, destMask;
        cap.read(frame);

        if (frame.empty()) {
            cout << "Empty Frame!" << endl;
            break;
        }

        // creates a mask from HSV values and circle detection for the object
        vector<Vec3f> circles;
        filterImage(&frame, &mask, lowerBoundObject, upperBoundObject, circles, true);

        // creates a mask from HSV values and circle detection for the destination
        vector<Vec3f> destCircles;
        filterImage(&frame, &destMask, lowerBoundDest, upperBoundDest, destCircles, false);

        // finds Contours for the Object
        vector<vector<Point> > contours;
        findContours(mask.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

        // finds Contours for the Destination
        vector<vector<Point> > destContours;
        findContours(destMask.clone(), destContours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

        // detects the object and draws to the frame
        // gives the center and radius of the object
        detectObject(&frame, circles, contours, &objectCenter, prev_objectCenter, &objectRadius, prev_objectRadius, true, &isOffscreen); 
        prev_objectCenter = objectCenter; 
        prev_objectRadius = objectRadius;

        detectObject(&frame, destCircles, destContours, &destCenter, prev_destCenter, &destRadius, prev_destRadius, false, &isOffscreen);
        prev_destCenter = destCenter;
        prev_objectRadius = objectRadius;

        int pt_size = objectPoints.size();
        int obpt_size = destPoints.size();
        int objectRadii_size = objectRadii.size();
        int destRadii_size = destRadii.size();

        // add object center points to queue
        objectPoints.push_back(objectCenter);
        // add destination center points to queue
        destPoints.push_back(destCenter);
        // add radius of object to queue
        objectRadii.push_back(objectRadius);
        // add radius of dest ro queue
        destRadii.push_back(destRadius);

        // draw line to frame from point queue of object movement
        for (int i = 1; i < pt_size; i++) {
            line(frame, objectPoints[i - 1], objectPoints[i], Scalar(43,231,123), 6);
        }

        // detects direction of object movement
        detectDirection(&frame, objectPoints, pt_size, &direction);

        // get averaged center points from object
        Point2f avgCenterPoint = getAveragePoint(objectPoints, pt_size);

        // get averaged center points from destination
        Point2f avgDestPoint = getAveragePoint(destPoints, obpt_size);

        // gets average radius of object
        float avgObjectRadius = getAverageRadius(objectRadii, objectRadii_size);

        // gets average radius of destination object
        float avgDestRadius = getAverageRadius(destRadii, destRadii_size);

        // need to calculate angle and driveDistance
        float angle = 0.0;
        float dx = abs(avgCenterPoint.x - avgDestPoint.x);
        float dy = abs(avgCenterPoint.y - avgDestPoint.y);
        float centerDistance = sqrt(dx*dx + dy*dy);
        float driveDistance = centerDistance - avgObjectRadius - avgDestRadius;

        if (debugMode) {
            cout << endl;
            cout << "distance left: " << driveDistance << endl;
            cout << "offscreen: " << isOffscreen << endl;
            cout << "object point: " << "(" << avgCenterPoint.x << ", " << avgCenterPoint.y << ")" << endl;
            cout << "object radius: " << avgObjectRadius << endl;
            cout << "dest point: " << "(" << avgDestPoint.x << ", " << avgDestPoint.y << ")" << endl;
            cout << "dest radius: " << avgDestRadius << endl;
            cout << "dircetion: " << direction << endl;
            cout << endl;
        }

        string outputCommands[3];

        MaxwellStatechart(
            driveDistance,          // distance from object to destination
            isOffscreen,            // if Object is isOffscreen
            avgCenterPoint.x,       // x point of Object
            avgCenterPoint.y,       // y point of Object
            avgObjectRadius,        // radius of Object
            avgDestPoint.x,         // x point of Destination
            avgDestPoint.y,         // y point of Destination
            avgDestRadius,          // radius of destination
            direction,              // direction object is moving
            outputCommands          // output 
        );

        // pop points queue
        if (pt_size >= MAXQUEUESIZE) {
            objectPoints.pop_front();
        }

        if (obpt_size >= MAXQUEUESIZE) {
            destPoints.pop_front();
        }

        // pop objectRadii queue
        if (objectRadii_size >= MAXQUEUESIZE) {
            objectRadii.pop_front();
        }

        // pop destRadii queue
        if (destRadii_size >= MAXQUEUESIZE) {
            destRadii.pop_front();
        }

        imshow("drawing", frame);

        if (waitKey(30) >= 0) {
            break;
        }


        // for testing, input data
        // char buffer[256];
        char data[10];
        memset(data, 0, strlen(data));
        char dist_angle[10];
        memset(dist_angle, 0, strlen(dist_angle));
        char percentSpeed[10];
        memset(percentSpeed, 0, strlen(percentSpeed));

        outputCommands[0].copy(data, outputCommands[0].size());
        outputCommands[1].copy(dist_angle, outputCommands[1].size());
        outputCommands[2].copy(percentSpeed, outputCommands[2].size());

        // printf("Please enter data: ");
        // memset(buffer, 0, 256);
        // fgets(buffer, 255, stdin);
        // memcpy(data, outputCommands[0], strlen(buffer));
        // printf("Please enter dist/angle: ");
        // memset(buffer, 0, 256);
        // fgets(buffer, 255, stdin);
        // memcpy(dist_angle, outputCommands[1], strlen(buffer));
        // printf("Please enter percent Speed: ");
        // memset(buffer, 0, 256);
        // fgets(buffer, 255, stdin);
        // memcpy(percentSpeed, outputCommands[2], strlen(buffer));

        // call to motionTrack uncomment when using
        // string outputCommands[3];
        // analyzeVideo(outputCommands);

        // data = outputCommands[0];
        // dist_angle = outputCommands[1];
        // percentSpeed = outputCommands[2];


        // create packet 
        char packet[256];
        memset(packet, 0, strlen(packet));
        packMessage(data, dist_angle, percentSpeed, packet);

        // send data packet
        n = write(sockfd, packet, strlen(packet) + 1);

    //     if (n < 0) {
    //         error("ERROR writing to socket");
    //     }
    //     memset(buffer, 0, 256);
    //     n = read(sockfd,buffer,255);

    //     if (n < 0) {
    //         error("ERROR reading from socket");
    //     }

    //     if (strcmp(buffer, "done") == 0) {
    //         break;
    //     }
    }
    close(sockfd);
    return 0;
}

















// int main(int argc, char *argv[]) {
//     int sockfd, portno, n;
//     struct sockaddr_in serv_addr;
//     struct hostent *server;

//     char buffer[256];

//     char *defaultIp;
//     int defaultPort;

//     if (argv[1] == NULL) {
//         printf("%s %s %s %d\n", "Using default ip: ", ip, " and default port: ", PORT);
//         defaultIp = ip;
//         defaultPort = PORT;
//     } else {
//         defaultIp = argv[1];
//         if (argv[2] == NULL) {
//             printf("Using default port: 51717...\n");
//             defaultPort = PORT;
//         } else {
//             defaultPort = atoi(argv[2]);
//         }
//     }

//     //Set up socket
//     portno = defaultPort;
//     sockfd = socket(AF_INET, SOCK_STREAM, 0);

//     if (sockfd < 0) {
//         error("ERROR opening socket");
//     }

//     server = gethostbyname(defaultIp);

//     if (server == NULL) {
//         fprintf(stderr,"ERROR, no such host\n");
//         exit(0);
//     }

//     memset((char *) &serv_addr, 0, sizeof(serv_addr));
//     serv_addr.sin_family = AF_INET;
//     memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
//     serv_addr.sin_port = htons(portno);

//     if (connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
//         error("ERROR connecting");
//     }

//     while (1) {

//         // for testing, input data
//         char buffer[256];
//         char data[10];
//         char dist_angle[10];
//         char percentSpeed[10];

//         printf("Please enter data: ");
//         memset(buffer, 0, 256);
//         fgets(buffer, 255, stdin);
//         memcpy(data, buffer, strlen(buffer));
//         printf("Please enter dist/angle: ");
//         memset(buffer, 0, 256);
//         fgets(buffer, 255, stdin);
//         memcpy(dist_angle, buffer, strlen(buffer));
//         printf("Please enter percent Speed: ");
//         memset(buffer, 0, 256);
//         fgets(buffer, 255, stdin);
//         memcpy(percentSpeed, buffer, strlen(buffer));

//         // call to motionTrack uncomment when using
//         // string outputCommands[3];
//         // analyzeVideo(outputCommands);

//         // data = outputCommands[0];
//         // dist_angle = outputCommands[1];
//         // percentSpeed = outputCommands[2];

//         // create packet 
//         char packet[256];
//         memset(packet, 0, strlen(packet));
//         packMessage(data, dist_angle, percentSpeed, packet);

//         // send data packet
//         n = write(sockfd, packet, strlen(packet) + 1);

//         if (n < 0) {
//             error("ERROR writing to socket");
//         }
//         memset(buffer, 0, 256);
//         n = read(sockfd,buffer,255);

//         if (n < 0) {
//             error("ERROR reading from socket");
//         }

//         if (strcmp(buffer, "done") == 0) {
//             break;
//         }
//     }
//     close(sockfd);
//     return 0;
// }