//Author Joseph Peters
//code basis from https://beej.us/guide/bgnet/html/#a-simple-stream-server

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define MAXDATASIZE 32 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char** argv)
{
	if (argc != 3) {
        fprintf(stderr,"Usage: ./client <hostname> <port>\n");
        exit(1);
    }

    int sockfd, numbytes;  
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }
    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure

	char input[MAXDATASIZE];
	char* result;

    printf("\nReady for user input. Type \";;;\" to disconnect and exit.\n\n");

	while(1){
		memset(input, '\0', MAXDATASIZE);
		printf("You> ");
		if((result = fgets(input, MAXDATASIZE, stdin)) == NULL){
			perror("readline");
		}

		if(strncmp(input, ";;;", 3) == 0) break;

		// if (send(sockfd, input, strlen(input), 0) == -1)
        //     perror("send");

		// if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
		// 	perror("recv");
		// 	exit(1);
		// }
        // buf[numbytes] = '\0';
        if (send(sockfd, " ", 1, 0) == -1)
            perror("send");
        if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
            perror("recv");
            exit(1);
        }
        buf[numbytes] = '\0';
		printf("Srv> %s ", buf);
        while(1){
            if (send(sockfd, input, strlen(input), 0) == -1)
                perror("send");

            if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
                perror("recv");
                exit(1);
            }
            buf[numbytes] = '\0';
            printf("%s", buf);
            if(input[strlen(input) - 1] == '\n') break;
            fgets(input, MAXDATASIZE, stdin);
        }
        if (send(sockfd, ".", 1, 0) == -1)
            perror("send");
	}

    close(sockfd);

    return 0;
}