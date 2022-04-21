//Author: Joseph Peters
//code basis from https://beej.us/guide/bgnet/html/#a-simple-stream-server

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>
#include <dirent.h>

#define BACKLOG 10   // how many pending connections queue will hold

#define MAXDATASIZE 1024

void iWant(int sockfd, char* filename);
void uTake(int sockfd, char* filename);

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void getUpperString(char* s){
	while(*s){
		*s = toupper(*s);
		s++;
	}
}

int main(int argc, char** argv){
	if(argc < 2) {
		printf("Usage: ./server <port>\nor\n\t./server <port> <ip_version_number (4 or 6)>\n");
		exit(1);
	}
	char* PORT = argv[1];
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if(argc == 3){
        if(strcmp(argv[2], "4") == 0){
            hints.ai_family = AF_INET;
        }
        else if(strcmp(argv[2], "6") == 0){
            hints.ai_family = AF_INET6;
        }
        else {
            printf("Invalid ip version. Defaulting to any\n");
        }
    }

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }

    // get the pointer to the address itself,
    // different fields in IPv4 and IPv6:
    void *addr;
    if (p->ai_family == AF_INET) { // IPv4
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
        addr = &(ipv4->sin_addr);
    } else { // IPv6
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
        addr = &(ipv6->sin6_addr);
    }
    char ipstr[INET6_ADDRSTRLEN];
    void* rt;
    if((rt = inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr)) == NULL){
        perror("ntop:");
    };

	printf("%s is running on port %s\n", ipstr, PORT);

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);

        if (!fork()) { // this is the child process
            close(sockfd); // child doesn't need the listener
			int bytesReceived = -1;
			char buf[MAXDATASIZE];

            while(1) {
                // get command
                bytesReceived = recv(new_fd, buf, MAXDATASIZE-1, 0);
                if (bytesReceived == -1) {
                    perror("recv");
                    exit(1);
                } else if (bytesReceived == 0) {
                    break;
                }

                // parse command
                buf[bytesReceived] = '\0';
                printf("received the following message from client:\n\n");
                printf("    \"%s\"\n\n", buf);

                char filename[MAXDATASIZE];
                // memset(fileDir, '\0', MAXDATASIZE);
                strncpy(filename, buf+strcspn(buf, " ")+1, MAXDATASIZE);
                
                if (strncmp(buf, ";;;", 3) == 0) {
                    break;
                } else if (strncmp(buf, "iWant", 5) == 0) {
                    iWant(new_fd, filename);
                } else if (strncmp(buf, "uTake", 5) == 0) {
                    uTake(new_fd, filename);
                } else {
                    printf("That just ain't right!\n");
                }
            }
			// while((recieved = recv(new_fd, buf, MAXDATASIZE - 1, 0)) != 0){
            //     messageCount++;
            //     int subCount = 0;
            //     char countString[10];
            //     sprintf(countString, "%d", messageCount);
            //     if (send(new_fd, countString, 10, 0) == -1)
            //             perror("send");
            //     while (1){
            //         recieved = recv(new_fd, buf, MAXDATASIZE - 1, 0);
            //         buf[recieved] = '\0';
            //         if(recieved == 1 && buf[0] == '.') break;
            //         printf("%s sent: %s", s, buf);
            //         if(buf[recieved - 1] != '\n') printf("\n");
            //         getUpperString(buf);
            //         if (send(new_fd, buf, recieved, 0) == -1)
            //             perror("send");
            //         subCount++;
            //         printf("sending message %d.%d to %s: %s\n", messageCount, subCount, s, buf);
            //     }
			// }
            printf("server: %s closed connection\n", s);
            close(new_fd);
            exit(0);
        }
        close(new_fd);  // parent doesn't need this
    }

    return 0;
}

void iWant(int sockfd, char* filename) {
    // Check for file existance and open
    printf("Client asking for file: %s", filename);
    FILE* fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("File not found: sending error message");
        if (send(sockfd, "FAIL\0", 5, 0) == -1) perror("send");
        return;
    }else{
        if (send(sockfd, "OK\0", 3, 0) == -1) {
            perror("send");
        }
    }

    // Send file to client
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
    strcpy(fileData, ".\0");
    if (send(sockfd, fileData, strlen(fileData), 0) == -1)
        perror("send");

    fclose(fp);

    printf("    file transfer of %d bytes to client complete\n", totalBytes);
}

void uTake(int sockfd, char* filename) {
    int bytesReceived = -1;
    int dirLen = -1;
    char buffer[MAXDATASIZE];
    // send OK message
    if (send(sockfd, "OK\0", 3, 0) == -1) {
        perror("send");
    }

    // get directory to save to
    bytesReceived = recv(sockfd, buffer, MAXDATASIZE-1, 0);
    if (bytesReceived == -1) {
        perror("recv");
        exit(1);
    }
    DIR* dir;
    printf("Received directory: %s\n", buffer);
    buffer[bytesReceived] = '\0';
    if(strcmp(buffer, "\n") == 0) {
        strcpy(buffer, "received_files");
        buffer[15] = '\0';
    }
    dir = opendir(buffer);
    dirLen = strlen(buffer);

    // send 'no' if directory nonexistent, 'OK' if exists
    if(dir == NULL) {
        if (send(sockfd, "no\0", 3, 0) == -1) {
            perror("send");
        }
        return;
    } else {
        if (send(sockfd, "OK\0", 3, 0) == -1) {
            perror("send");
        }
    }

    strcat(strcat(buffer, "/"), filename);
    buffer[dirLen+1+strlen(filename)] = '\0';

    // receive file
    FILE* fp = fopen(buffer, "wb");

    while(1) {
        char fileData[MAXDATASIZE];
        bytesReceived = recv(sockfd, fileData, MAXDATASIZE, 0);
        printf("Received %d bytes\n", bytesReceived);
        if (bytesReceived == -1) {
            perror("recv");
            exit(1);
        }
        if (fileData[bytesReceived - 1] == '.') {
            fwrite(fileData, sizeof(char), bytesReceived - 1, fp);
            printf("    finished receiving file.\n");
            fclose(fp);
            return;
        } else {
            fwrite(fileData, sizeof(char), bytesReceived, fp);
        }
    }


}