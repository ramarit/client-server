//
//  server.cpp
//
//  Created by Ryan Amarit on 5/16/16.
//  Copyright © 2016 Ryan Amarit. All rights reserved.
//

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <time.h>
#include <pthread.h>
#include <sstream>
#include <semaphore.h>
#include <string>
#include <vector>
#include <algorithm>
#include <ostream>
#include <arpa/inet.h>

#pragma pack(1) // this helps to pack the struct to 5-bytes
struct player
{
    std::string name;
    int score;
    int finish;
    player(std::string n, int s, int f) : name(n), score(s), finish(f) {}
};
#pragma pack(0) // turn packing off

std::vector<player> high_scores;

pthread_mutex_t m_lock;
int finishOrder = 0;


bool leaderSort(const player&lhs, const player&rhs)
{
    if (lhs.score != rhs.score)
        return lhs.score < rhs.score;
    return lhs.finish < rhs.finish;
}

//called when a system call fails.
//displays a message about the error on stderr and then aborts the program.
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void add_score(std::string name, int score, int finishOrder)
{
    high_scores.push_back(player(name, score, finishOrder));
    sort(high_scores.begin(), high_scores.end(), leaderSort);
}
void * Guess(void * newsocket);


int main(int argc, const char * argv[]) {
     std::stringstream stream;
    int sockfd, portno;
    //stores the size of the address of the client. This is needed for the accept system call.
    socklen_t clilen;
    
    add_score("", 1000, 10000);
    add_score("", 1000, 10001);
    add_score("", 1000, 10002);
    struct sockaddr_in serv_addr, cli_addr;
    //return value for the read() and write() calls
    //long int n;
    
    //Code to display error message if the user fails to pass port number
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        std::cout << stream.str();
        stream.str("");
        
        exit(1);
    }
 
    // creates a new socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    
    //sets all values in a buffer to zero
    bzero((char *) &serv_addr, sizeof(serv_addr));
    
    //port number on which the server will listen for connections
    //atoi() converts this from a string of digits to an integer
    portno = atoi(argv[1]);
    
    //Setting fields of serv_addr, which is of type struct sockaddr_in
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    
    //binds a socket to an address
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    
    //allows the process to listen on the socket for connections
    listen(sockfd,5);
    
    //The accept() causes the process to block until a client connects to the server
    //returns a new file descriptor, and all communication on this connection
    //should be done using the new file descriptor
    clilen = sizeof(cli_addr);
    pthread_t guess;
    while(true)
    {
        int newsockfd = accept(sockfd,
                           (struct sockaddr *) &cli_addr,
                           &clilen);
        if (newsockfd < 0)
            error("ERROR on accept");
        int guessThread = pthread_create(&guess, NULL, Guess, (void *)&newsockfd);
        if (guessThread != 0)
        {
            stream << "thread error, exiting..." << std::endl;
            std::cout << stream.str();
            stream.str("");
            return 0;
        }
        stream << "\nUser Joining Server: Thread Created" << std::endl;
        std::cout << stream.str();
        stream.str("");
    }
}
//thread below
void * Guess(void * newsocket)
{
    int finish;
    std::stringstream stream;
    //char buffer[256];
    char sendBuff[10000];
    char recvBuff[10000];
    long int n;
    
    
    memset(sendBuff, '0', sizeof(sendBuff));
    int newsockfd = *(int *)newsocket;
    
    memset(recvBuff, '\0', sizeof(recvBuff));
    n = recv(newsockfd, recvBuff, sizeof(recvBuff),0);
    if (n < 0) error("ERROR reading from socket");
    printf( "\nClient's name: %s\n", recvBuff);
    std::string name(recvBuff);

    //get random number and store answer into array
    srand(time(NULL));
    int answer = rand()%10000;
    int answerArr[4];
    int answerTrack = answer;
    int answerIndex = 3;
    while (answerIndex >= 0)
    {
        answerArr[answerIndex] = answerTrack%10;
        answerTrack /= 10;
        answerIndex--;
    }
    
    //print answer
    printf("%s's answer to guess: %d%d%d%d\n", name.c_str(), answerArr[0],answerArr[1],answerArr[2],answerArr[3]);
    //std::cout << stream.str();
    //stream.str("");
    int guess = -1;
    int difference = -1;
    int closeness = 0;
    int guessCount = 0;
    while (difference != 0)
    {
        n = read(newsockfd, &guess, sizeof(guess));
        if (n < 0) error("ERROR reading from socket");
        int converted_guess = ntohl(guess);
   
        int guessArr[4];
        int guessTrack = converted_guess;
        int guessIndex = 3;
        //iterate through the guess int and put user guess into array
        while (guessIndex >= 0)
        {
            guessArr[guessIndex] = guessTrack%10;
            guessTrack /= 10;
            guessIndex--;
        }
        
        stream << printf("%s' guess: %d%d%d%d\n", name.c_str(), guessArr[0],guessArr[1],guessArr[2],guessArr[3]);
        stream.str("");
        //get difference from guess
        int differenceArr[4];
        for (int i = 0; i < 4; i++)
        {
            if (guessArr[i] >= answerArr[i])
            {
                differenceArr[i] = guessArr[i] - answerArr[i];
            }
            else
            {
                differenceArr[i] = answerArr[i] - guessArr[i];
            }
            closeness += differenceArr[i];
        }
        if (closeness == 0)
        {
            difference = closeness;
            int converted_difference = htonl(difference);
            n = write(newsockfd, &converted_difference, sizeof(converted_difference));
            if (n < 0) error("ERROR reading from socket");
        }
        else
        {
            int converted_closeness = htonl(closeness);
            n = write(newsockfd, &converted_closeness, sizeof(converted_closeness));
            if (n < 0) error("ERROR reading from socket");
        }
        closeness = 0;
        guessCount++;
    }
    int converted_guessCount = htonl(guessCount);
    n = write(newsockfd, &converted_guessCount, sizeof(converted_guessCount));
    if (n < 0) error("ERROR reading from socket");
    //lock mutex
    pthread_mutex_lock(&m_lock);
    finishOrder += 1;
    finish = finishOrder;
    add_score(name, guessCount,finish);
 
    //write to leader board
    for (int i = 0; i < 3; i++)
    {
        if (high_scores[i].score == 1000)
        {
            memset(sendBuff, '\0', sizeof(sendBuff));
            sprintf(sendBuff,"- -");
            n = send(newsockfd, sendBuff, strlen(sendBuff), 0);
            if (n < 0) error("ERROR writing to socket");
        }
        else
        {
            memset(sendBuff, '\0', sizeof(sendBuff));
            sprintf(sendBuff,"%s %d" , high_scores[i].name.c_str(), high_scores[i].score);
            n = send(newsockfd, sendBuff, strlen(sendBuff), 0);
            if (n < 0) error("ERROR writing to socket");
        }
        memset(recvBuff, '\0', sizeof(recvBuff));
        n = recv(newsockfd, recvBuff, sizeof(recvBuff),0);
        if (n < 0)
            error("ERROR reading from socket");
    }
    //unlock mutex
    pthread_mutex_unlock(&m_lock);
    close(newsockfd);
    return 0;
}

