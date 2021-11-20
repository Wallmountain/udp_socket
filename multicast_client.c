#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define localIP "10.0.2.15"

struct sockaddr_in localsock;
struct ip_mreq group;
int sockfd, len;
char buffer[1024] = {0};

void error (char *msg){
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]){ 

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0))< 0)
		error("Opening datagram socket error");
    else printf("Opening datagram socket. . . .OK.\n");

	/* Enable SO_REUSEADDR to allow multiple instances of this */
	/* application to receive copies of the multicast datagrams. */
	int reuse = 1;
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse)) < 0)
		error("Setting SO_REUSEADDR error");
    else printf("Setting SO_REUSEADDR . . .OK.\n");
    
    // give recvfrom time limit 50ms
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 50000; // 50ms
    
	/* Bind to the proper port number with the IP address */
	/* specified as INADDR_ANY. */
	memset((char *) &localsock, 0, sizeof(localsock));
	localsock.sin_family = AF_INET;
	localsock.sin_port = htons(4321);
    localsock.sin_addr.s_addr =  INADDR_ANY;

	if(bind(sockfd, (struct sockaddr*)&localsock, sizeof(localsock)))
		error("Binding datagram socket error");
    else printf("Binding datagram socket . . .OK.\n");
    
    /* Join the multicast group 226.1.1.1 on the local 203.106.93.94 */
	/* interface. Note that this IP_ADD_MEMBERSHIP option must be */
	/* called for each local interface over which the multicast */
	/* datagrams are to be received. */
	group.imr_multiaddr.s_addr = inet_addr("226.1.1.1");
	group.imr_interface.s_addr = inet_addr(localIP);

	if(setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0)
		error("Adding multicast group error"); 
    else printf("Adding nulticasr group . . .OK.\n");
	
    /* Read from the socket. */
    len = read(sockfd, buffer, 1024);
    char filename[len+1];
    memcpy(filename, buffer, len);
    FILE *fp = fopen(filename,"w+"); // open file

    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    while ((len = read(sockfd, buffer, 1024))>0){
        fwrite(buffer, sizeof(char), len, fp); // write content to file 
        bzero(buffer, 1024);
    }

    // calculate thr file size
    fseek(fp, 0, SEEK_END);
    printf("finish recv\nsend file size : %ldKB\n",ftell(fp)/1024);

    fclose(fp);
    close(sockfd);

	return 0;
}
