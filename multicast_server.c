#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define localIP "10.0.2.15"

struct sockaddr_in groupsock;
struct in_addr localinterface;
int sockfd;
char *filename;

void error (char *msg){
    perror(msg);
    exit(1);
}

int main(int argc, char **argv){
    
    filename = argv[1];
    if(argc < 2)
        error("argv* has no portno");

    if(access(filename,0)<0)  // Check if there is this file
        error("no this file");

    // socket
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        error("socket error");
    else printf("Opening the datagram socket. . .OK.\n");

    /* Initialize the group sockaddr structure with a */
	/* group address of 225.1.1.1 and port 5555. */
	memset((char *) &groupsock, 0, sizeof(groupsock));
	groupsock.sin_family = AF_INET;
	groupsock.sin_addr.s_addr = inet_addr("226.1.1.1");
	groupsock.sin_port = htons(4321);

    /* Set local interface for outbound multicast datagrams. */
	/* The IP address specified must be associated with a local, */
	/* multicast capable interface. */
	localinterface.s_addr = inet_addr(localIP);
	if(setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localinterface, sizeof(localinterface)) < 0)
        error("Setting local interface error");
    else printf("Setting local interface. . .OK.\n");

    int len;
    char buffer[1024]= {0};

    // send filename to client
    memcpy(buffer, filename, strlen(filename));
    sendto(sockfd, buffer, 1024, 0, (struct sockaddr *) &groupsock, sizeof(groupsock));
    bzero(buffer, 1024);

    FILE *file_fp = fopen(filename, "r"); // open file
    while((len = fread(buffer, sizeof(char), 1024, file_fp)) > 0){
        // if there is data in file , send them to client.
        if (sendto(sockfd, buffer, len+1, 0, (struct sockaddr *) &groupsock, sizeof(groupsock)) < 0)
            error("send file failed!\n");
        bzero(buffer, 1024);
    }

    // calculate file size
    fseek(file_fp, 0, SEEK_END);
    if(len == 0)
        printf("finish send\nsend file size : %ldKB\n",ftell(file_fp)/1024);
    else if(len == -1)
         error("recv failed");

    fclose(file_fp);
    close(sockfd);

    return 0;
}
