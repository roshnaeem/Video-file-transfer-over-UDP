#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <unistd.h>
#include <strings.h>

// This will be the size of our payload/ buffer size
#define BUFFERT 471

int create_server_socket (int port);

struct sockaddr_in sock_serv,clt;

// THe structure of our packet
struct packet{
	int seq;
	char buf[BUFFERT];
};

//the main function
int main (int argc, char *argv[]){

    //declaration of various variables to be used later
	int fd, sfd;
	off_t count=0, n0,n1,n2,n3,n4, ackn; // long type
	char filename[256];
    unsigned int l=sizeof(struct sockaddr_in);

    //memory allocation to recieve packet
    struct packet * packet0 = malloc(sizeof(struct packet));
    int ackNo0, ackNo1, ackNo2, ackNo3, ackNo4;
    struct packet * packet_seq[5];
    int noOfPackets;
    int c;

    // Variable for the date
	time_t intps;
	struct tm* tmi;

    //check if correct number of arguments have been entered
	if (argc != 2){
		printf("Error usage : %s <port_serv>\n",argv[0]);
		return EXIT_FAILURE;
	}

    sfd = create_server_socket(atoi(argv[1]));

    //create a new file
	intps = time(NULL);
	tmi = localtime(&intps);
	bzero(filename,256);
	sprintf(filename,"clt.%d.%d.%d.%d.%d.%d",tmi->tm_mday,tmi->tm_mon+1,1900+tmi->tm_year,tmi->tm_hour,tmi->tm_min,tmi->tm_sec);
	printf("Creating the output file : %s\n",filename);

	//open the new created file

	if((fd=open(filename,O_CREAT|O_WRONLY|O_TRUNC,0600))==-1){
		perror("open fail");
		return EXIT_FAILURE;
	}

    //recieve the number of packets to be recieved and send ack
    n0=recvfrom(sfd,&noOfPackets,4,0,(struct sockaddr *)&clt,&l);
    ackn=sendto(sfd,&noOfPackets,4,0,(struct sockaddr*)&clt,l);
    ackn=sendto(sfd,&noOfPackets,4,0,(struct sockaddr*)&clt,l);
    ackn=sendto(sfd,&noOfPackets,4,0,(struct sockaddr*)&clt,l);
    ackn=sendto(sfd,&noOfPackets,4,0,(struct sockaddr*)&clt,l);

    printf("no. of packets to be recieved: %d \n", noOfPackets);

    noOfPackets+=1;
    //array to keep track of duplicates
    int allSeq[noOfPackets];

    for (int i=0; i<noOfPackets; i++){
    	allSeq[i]=0;
    }
    //array of packets recieved in a window
    struct packet allPackets[5];
    int sizeOfPacket[5];

    n0=1;
    int recFromWindow=0;

   // off_t alln[noOfPackets];

    //this while loop will keep running until 0 is not recieved indicating end of file
    	while(n0){
    	//recieve packet
    		n0=recvfrom(sfd,packet0,sizeof(*packet0),0,(struct sockaddr *)&clt,&l);
        	ackNo0=packet0->seq;
        	//if duplicate packet is not recieved
        	if (allSeq[ackNo0]==0 && n0!=0){
        		allPackets[ackNo0%5]=*packet0;
        		allSeq[ackNo0]=1;
        		sizeOfPacket[ackNo0%5]=n0-4;

        		recFromWindow+=1;
        		ackn=sendto(sfd,&ackNo0,4,0,(struct sockaddr*)&clt,l);
    			printf("%ld of data received \n",n0);
			printf("ack sequence no recieved: %d \n", packet0->seq);
			count+=n0;
        	}
        	else{
        		printf("duplicate packet recieved of packet: %d \n", ackNo0);
        		ackn=sendto(sfd,&ackNo0,4,0,(struct sockaddr*)&clt,l);
        	}

        	//if all packets of current winodw are recieved.
        	//write the 5 packets in order into the file if they are not empty
        	if(recFromWindow==5){
        		for(int i=0; i<5; i++){
        			if(sizeOfPacket[i]!=0){
        				write(fd,allPackets[i].buf,sizeOfPacket[i]);
        			}
        		}
        			for(int i=0; i<5; i++){
        				sizeOfPacket[i]=0;
        			}

        		recFromWindow=0;

        	}

    	}
    	//write the 5 packets in order into the file if they are not empty (for the remaining)
    	for(int i=0; i<5; i++){
        	if(sizeOfPacket[i]!=0){
        		write(fd,allPackets[i].buf,sizeOfPacket[i]);
        	}
        }

	printf("Number of Bytes transferred : %ld \n",count);

    //close socket
    close(sfd);

    //close file
    close(fd);
	return EXIT_SUCCESS;
}


int create_server_socket (int port){
    int l;
	int sfd;

	sfd = socket(AF_INET,SOCK_DGRAM,0);
	if (sfd == -1){
        perror("socket fail");
        return EXIT_FAILURE;
	}

    //preparation of the address of the destination socket
	l=sizeof(struct sockaddr_in);
	bzero(&sock_serv,l);

	sock_serv.sin_family=AF_INET;
	sock_serv.sin_port=htons(port);
	sock_serv.sin_addr.s_addr=htonl(INADDR_ANY);

	//Assign an identity to the socket
	if(bind(sfd,(struct sockaddr*)&sock_serv,l)==-1){
		perror("bind fail");
		return EXIT_FAILURE;
	}
    return sfd;
}
