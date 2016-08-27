//
//  client.cpp
//  hw4
//
//  Created by Ryan Amarit on 5/16/16.
//  Copyright © 2016 Ryan Amarit. All rights reserved.
//

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sstream>
#include <vector>
//#include <arpa/inet.h>
#include <limits>

using namespace std;
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

#pragma pack(1) // this helps to pack the struct to 5-bytes
struct player
{
    std::string name;
    int score;
    int finish;
};
#pragma pack(0) // turn packing off


int main(int argc, char *argv[])
{
    struct player high_scores;
    stringstream stream;
    int sockfd, portno;
    long n;
    //const int MAX_BUF = 10000;
    struct sockaddr_in serv_addr; //contain the address of the server to which we want to connect
    struct hostent *server; //pointer to a structure of type hostent
    
    //char buffer[256];
    char sendBuff[10000];
    char recvBuff[10000];
    if (argc < 3) {
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
        cout << stream.str();
        stream.str("");
        exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    
    //argv[1] contains the name of a host on the Internet
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    
    //sets the fields in serv_addr
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);
    
    //connect() is called by the client to establish connection to the server
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");
    stream << "Welcome to Number Guessing Game" << endl;
    cout << stream.str();
    stream.str("");

    //Prompts the user to enter a message, uses fgets to read the message from stdin,
    //writes the message to the socket, reads the reply from the socket,
    //and displays this reply on the screen
    string clientName;
    stream << "Please enter your name: ";
    cout << stream.str();
    cin >> clientName;
    stream.str("");
    memset(sendBuff, '\0', sizeof(sendBuff));
    sprintf(sendBuff, "%s", clientName.c_str());
    send(sockfd, sendBuff, strlen(sendBuff), 0);
    
    //client guess
    int difference = -1;
    int guess = -1;
    int guessCount = 0;
    while (difference != 0)
    {
        stream << "Please enter your guess(0-9999): " << endl;
        cout << stream.str();
        stream.str("");
        cin >> guess;
        while(cin.fail() || (guess < 0 || guess > 9999))
        {
            cin.clear();
            cin.ignore(numeric_limits<int>::max(), '\n');
            cout << "Bad Entry, Enter again: ";
            cin >> guess;
            
        }
        int converted_number = htonl(guess);
        n = write(sockfd, &converted_number, sizeof(converted_number));
        if (n < 0)
            error("ERROR writing to socket");

        n = read(sockfd, &difference, sizeof(difference));
        if (n < 0)
            error("ERROR reading from socket");
        int converted_difference = ntohl(difference);
        printf("Closeness: %d\n",converted_difference);
    }
    n = read(sockfd, &guessCount,sizeof(guessCount));
    if (n < 0)
        error("ERROR reading from socket");
    int converted_guessCount = ntohl(guessCount);
    stream << "Congratulations! It took " << converted_guessCount << " turn(s) to guess the number" << endl;
    cout << stream.str();
    stream.str("");

    
    stream << "Leaderboard:" << endl;
    cout << stream.str();
    stream.str("");
    for (int i = 0; i < 3; i++)
    {   
        memset(recvBuff, '\0', sizeof(recvBuff));
        n = recv(sockfd, recvBuff, sizeof(recvBuff),0);
        if (n < 0)
            error("ERROR reading from socket");

        
        memset(sendBuff, '\0', sizeof(sendBuff));
        sprintf(sendBuff,"\n");
        n = send(sockfd, sendBuff, strlen(sendBuff), 0);
        if (n < 0) error("ERROR writing to socket");
        printf("%d. %s\n",i+1, recvBuff);
    }
    close(sockfd);
    return 0;
}
