//Author Joseph Peters, Dalton Julian
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
#include <dirent.h>
#include <arpa/inet.h>

#define MAXDATASIZE 1024 // max number of bytes we can get at once 

void iWant(int sockfd, char* input, char* buffer);
void uTake(int sockfd, char* input, char* buffer, char* fileDir);

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

    int sockfd;  
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
        // read input from user
		memset(input, '\0', MAXDATASIZE);
        memset(buf, '\0', MAXDATASIZE);
		printf("> ");
		if((result = fgets(input, MAXDATASIZE-1, stdin)) == NULL){
			perror("readline");
		}
 
        input[strcspn(input, "\n")] = '\0';
        char fileDir[MAXDATASIZE];
        // memset(fileDir, '\0', MAXDATASIZE);

        strncpy(fileDir, input+strcspn(input, " ")+1, MAXDATASIZE);

        // detect command
		if(strncmp(input, ";;;", 3) == 0) {
            break;
        } else if (strncmp(input, "iWant", 5) == 0) {
            iWant(sockfd, input, buf);
        } else if (strncmp(input, "uTake", 5) == 0) {
            uTake(sockfd, input, buf, fileDir);
        } else {
            printf("That just ain't right!\n");
        }

		// if (send(sockfd, input, strlen(input), 0) == -1)
        //     perror("send");

		// if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
		// 	perror("recv");
		// 	exit(1);
		// }
        // buf[numbytes] = '\0';
        // if (send(sockfd, " ", 1, 0) == -1)
        //     perror("send");
        // if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        //     perror("recv");
        //     exit(1);
        // }
        // buf[numbytes] = '\0';
		// printf("Srv> %s ", buf);
        // while(1){
        //     if (send(sockfd, input, strlen(input), 0) == -1)
        //         perror("send");

        //     if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        //         perror("recv");
        //         exit(1);
        //     }
        //     buf[numbytes] = '\0';
        //     printf("%s", buf);
        //     if(input[strlen(input) - 1] == '\n') break;
        //     fgets(input, MAXDATASIZE, stdin);
        // }
        // if (send(sockfd, ".", 1, 0) == -1)
        //     perror("send");
	}

    close(sockfd);

    return 0;
}

void iWant(int sockfd, char* input, char* buffer) {
    // get filename
    char filename[MAXDATASIZE];
    char* filePart = strrchr(input, ' ')+1;
    if (strcspn(filePart, "/") != strlen(filePart)) {
        strcpy(filename, strrchr(filePart, '/')+1);
    } else {
        strcpy(filename, filePart);
    }
    strcat(filename, "\0");
    

    // send command to server
    if (send(sockfd, input, strlen(input), 0) == -1) {
        perror("send");
    }

    // receive response from server, error if file/directory not exist
    int bytesReceived = -1;
    bytesReceived = recv(sockfd, buffer, MAXDATASIZE-1, 0);
    if (bytesReceived == -1) {
        perror("recv");
        exit(1);
    }
    buffer[bytesReceived] = '\0';
    if (strcmp(buffer, "OK") != 0) {
        printf("What you talkin bout Willis? I ain't seen that file nowhere!\n");
        return;
    }

    // prompt for directory to save file to on client
    while(1) {
        char* result;
        DIR* dir;
        printf("What directory would you like to save this file?\n");
        printf("> ");
        if((result = fgets(input, MAXDATASIZE-1, stdin)) == NULL){
            perror("readline");
        }
        if(strcmp(input, "\n") == 0) {
            strcpy(input, "received_files");
            input[15] = '\0';
        } else {
            input[strcspn(input, "\n")] = '\0';
        }
        dir = opendir(input);
        if(dir == NULL) {
            printf("That directory does not exist on the client.\n\n");
        } else {
            break;
        }
    }

    // receive file from server

    int dirLen = strlen(input);
    strcat(strcat(input, "/"), filename);
    input[dirLen+1+strlen(filename)] = '\0';

    FILE* fp = fopen(input, "wb");

    int totalBytes = 0;
    while(1) {
        char fileData[MAXDATASIZE];
        bytesReceived = recv(sockfd, fileData, MAXDATASIZE, 0);
        if (bytesReceived == -1) {
            perror("recv");
            exit(1);
        }
        totalBytes += bytesReceived;
        if (fileData[bytesReceived - 1] == '.') {
            totalBytes -= 1;
            fwrite(fileData, sizeof(char), bytesReceived - 1, fp);
            fclose(fp);
            return;
        }else{
            fwrite(fileData, sizeof(char), bytesReceived, fp);
        }
    }

    // print OK message
    fclose(fp);
    printf("    file transfer of %d bytes complete and placed in %s\n", totalBytes, input);
}


void uTake(int sockfd, char* input, char* buffer, char* fileDir) {
    // open file, error if not exists
    int bytesReceived = -1;
    FILE *fp = fopen(fileDir, "rb");
    if (fp == NULL) {
        printf("What you talkin bout Willis? I ain't seen that file nowhere!\n");
        return;
    }

    // put together command for server with uTake <filename>
    char filename[MAXDATASIZE];
    memset(filename, '\0', MAXDATASIZE);
    if (strcspn(fileDir, "/") != strlen(fileDir)+1) {
        strcpy(filename, strrchr(fileDir, '/')+1);
    } else {
        strcpy(filename, fileDir);
    }
    strcat(filename, "\0");
    char command[MAXDATASIZE];
    strcpy(command, "uTake ");
    strcat(command, filename);
    command[6+strlen(filename)] = '\0';

    // send input to server to indicate command
    if (send(sockfd, command, strlen(command), 0) == -1) {
        perror("send");
    }

    // receive response from server
    bytesReceived = recv(sockfd, buffer, MAXDATASIZE-1, 0);
    if (bytesReceived == -1) {
        perror("recv");
        exit(1);
    }
    buffer[bytesReceived] = '\0';

    // prompt for directory to save to on server
    char* result;
    printf("What directory on the server would you like to save this file?\n");
    printf("> ");
    if((result = fgets(input, MAXDATASIZE-1, stdin)) == NULL){
        perror("readline");
    }
    if(!(strlen(input) == 1 && input[0] == '\n')){
        input[strcspn(input, "\n")] = '\0';
    }
    printf("read: %s of size %d\n", input, strlen(input));

    // send directory to server
    if (send(sockfd, input, strlen(input), 0) == -1) {
        perror("send");
    }

    // receive response about directory from server
    bytesReceived = recv(sockfd, buffer, MAXDATASIZE-1, 0);
    if (bytesReceived == -1) {
        perror("recv");
        exit(1);
    }
    buffer[bytesReceived] = '\0';
    if (strcmp(buffer, "OK") != 0) {
        printf("That directory doesn't exist on the server!\n");
        return;
    }

    // send file to server
    printf("    file transfer initiated...\n");
    char fileData[MAXDATASIZE];
    int bytesRead = -1;
    int totalBytes = 0;
    while(1){
        memset(fileData, '\0', MAXDATASIZE);
        bytesRead = fread(fileData, sizeof(char), MAXDATASIZE, fp); // if file end reached
        totalBytes += bytesRead;
        if (send(sockfd, fileData, bytesRead, 0) == -1)
            perror("send");
        if (bytesRead < MAXDATASIZE) {
            break;
        }
    }

    // send file-ended message to server
    memset(buffer, '\0', MAXDATASIZE);
    strcpy(buffer, ".\0");
    if (send(sockfd, buffer, strlen(buffer), 0) == -1)
        perror("send");

    // print OK message
    fclose(fp);
    printf("    file transfer of %d bytes to server complete and placed in %s\n", totalBytes, input);
}