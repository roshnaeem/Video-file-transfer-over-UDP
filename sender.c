#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>


#include <fcntl.h>
#include <unistd.h>
#include <strings.h>
#include <stdbool.h>
#include <string.h>


// This will be the size of our payload/ buffer size
#define BUFFERT 471



int create_client_socket (int port, char* ipaddr);

struct sockaddr_in sock_serv;


//The structure of our packet
struct packet{
	int seq;
	char buf[BUFFERT];
};

//This is the main function

int main (int argc, char**argv){


	//Declaration of all the variables to be used
    int sfd,fd;
    off_t count=0, m0,m1,m2,m3,m4,ackm,sz;//long
	long int n0,n1,n2,n3,n4;
    int l=sizeof(struct sockaddr_in);
	struct stat buffer;

	int seqNo;
	seqNo=0;
	int ackNo0,ackNo1,ackNo2,ackNo3,ackNo4;
	int recieved_ack[5]={};
	int recievedAcks, ackNoOfPackets, noOfPackets, duplicateAck;
	bool size_ack;
	size_ack=false;

	// These will be the expected acks
	int expAck0, expAck1, expAck2, expAck3, expAck4;

    //Check if user has entered the correct arguments
	if (argc != 4){
		printf("Error usage : %s <ip_serv> <port_serv> <filename>\n",argv[0]);
		return EXIT_FAILURE;
	}

    //Creating the UDP socket
    sfd=create_client_socket(atoi(argv[2]), argv[1]);

    //Open the file to be sent in readonly format
	if ((fd = open(argv[3],O_RDONLY))==-1){
		perror("open fail");
		return EXIT_FAILURE;
	}

//file size calculation
	if (stat(argv[3],&buffer)==-1){
		perror("stat fail");
		return EXIT_FAILURE;
	}
	else
		sz=buffer.st_size;

	//calculate the number of packets to be sent and send it.
	//Send it again after timeout of 1 second until ack is not recieved
	noOfPackets=(sz/BUFFERT)+1;
	while(!size_ack){
		m0=sendto(sfd,&noOfPackets,4,0,(struct sockaddr*)&sock_serv,l);
		ackm=recvfrom(sfd,&ackNoOfPackets,4,0,(struct sockaddr *)&sock_serv,&l);

		if(ackNoOfPackets==noOfPackets){
			size_ack=true;
		}
	}

	//Declare the 5 packets to be sent in a window
	struct packet packet0, packet1, packet2, packet3, packet4;

	//Read the file into the buffer of the first packet
    n0=read(fd,packet0.buf,BUFFERT);

    //Run this while loop until the file has not finished. i.e. till n0 is not 0.
	while(n0){
		if(n0==-1){
			perror("read fails");
			return EXIT_FAILURE;
		}

		//Array to keep track of which ack is not recieved
		recieved_ack[0]=0;
		recieved_ack[1]=0;
		recieved_ack[2]=0;
		recieved_ack[3]=0;
		recieved_ack[4]=0;
		//Number of acks recieved;
		recievedAcks=0;

		ackNo0=ackNo1=ackNo2=ackNo3=ackNo4=-1;
		expAck0=seqNo;
		packet0.seq=seqNo;
		seqNo+=1;
		expAck1=seqNo;
		packet1.seq=seqNo;
		seqNo+=1;
		expAck2=seqNo;
		packet2.seq=seqNo;
		seqNo+=1;
		expAck3=seqNo;
		packet3.seq=seqNo;
		seqNo+=1;
		expAck4=seqNo;
		packet4.seq=seqNo;
		seqNo+=1;

		//read file into buffers of rest of the packets
		n1=read(fd,packet1.buf,BUFFERT);
		n2=read(fd,packet2.buf,BUFFERT);
		n3=read(fd,packet3.buf,BUFFERT);
		n4=read(fd,packet4.buf,BUFFERT);
		m0=m1=m2=m3=m4=0;

		//variable to keep track of duplicate acks.
		duplicateAck=0;
		ackm=0;

		/* send the first packet
		after that send any non empty packet also*/

		m0=sendto(sfd,(struct packet*)&packet0,(n0+4),0,(struct sockaddr*)&sock_serv,l);
		if(n1!=0){
			m1=sendto(sfd,(struct packet*)&packet1,(n1+4),0,(struct sockaddr*)&sock_serv,l);
		}
		else{
			recievedAcks+=1;
			recieved_ack[1]=1;
		}
		if(n2!=0){
			m2=sendto(sfd,(struct packet*)&packet2,(n2+4),0,(struct sockaddr*)&sock_serv,l);
		}
		else{
			recievedAcks+=1;
			recieved_ack[2]=1;
		}
		if(n3!=0){
			m3=sendto(sfd,(struct packet*)&packet3,(n3+4),0,(struct sockaddr*)&sock_serv,l);
		}
		else{
			recievedAcks+=1;
			recieved_ack[3]=1;
		}
		if(n4!=0){
			m4=sendto(sfd,(struct packet*)&packet4,(n4+4),0,(struct sockaddr*)&sock_serv,l);
		}
		else{
			recievedAcks+=1;
			recieved_ack[4]=1;
		}

		if(m0==-1){
			perror("send error");
			return EXIT_FAILURE;
		}

		//Recieve the ack.
		ackm=recvfrom(sfd,&ackNo0,4,0,(struct sockaddr *)&sock_serv,&l);

		if(ackNo0 == expAck0 || ackNo0==expAck1 || ackNo0==expAck2 || ackNo0==expAck3 || ackNo0==expAck4){
			recieved_ack[ackNo0%5]=1;
			recievedAcks+=1;
			printf("ack recieved: %d \n", ackNo0);

		}
		else if((ackNo0 != expAck0 || ackNo0!=expAck1 || ackNo0!=expAck2 || ackNo0!=expAck3 || ackNo0!=expAck4) && ackm!=-1){
			printf("duplicate ack: %d \n", ackNo0);
			duplicateAck+=1;
		}

		ackm=0;
		ackm=recvfrom(sfd,&ackNo1,4,0,(struct sockaddr *)&sock_serv,&l);
		if(ackNo1 == expAck0 || ackNo1==expAck1 || ackNo1==expAck2 || ackNo1==expAck3 || ackNo1==expAck4){
			recieved_ack[ackNo1%5]=1;
			recievedAcks+=1;
			printf("ack recieved: %d \n", ackNo1);
		}
		else if((ackNo1 != expAck0 || ackNo1!=expAck1 || ackNo1!=expAck2 || ackNo1!=expAck3 || ackNo1!=expAck4) && ackm!=-1){
			printf("duplicate ack: %d \n", ackNo1);
			duplicateAck+=1;
		}

		ackm=0;
		ackm=recvfrom(sfd,&ackNo2,4,0,(struct sockaddr *)&sock_serv,&l);
		if(ackNo2 == expAck0 || ackNo2==expAck1 || ackNo2==expAck2 || ackNo2==expAck3 || ackNo2==expAck4){
			recieved_ack[ackNo2%5]=1;
			recievedAcks+=1;
			printf("ack recieved: %d \n", ackNo2);
		}
		else if((ackNo2 != expAck0 || ackNo2!=expAck1 || ackNo2!=expAck2 || ackNo2!=expAck3 || ackNo2!=expAck4) && ackm!=-1){
			printf("duplicate ack: %d \n", ackNo2);
			duplicateAck+=1;
		}

		ackm=0;
		ackm=recvfrom(sfd,&ackNo3,4,0,(struct sockaddr *)&sock_serv,&l);
		if(ackNo3 == expAck0 || ackNo3==expAck1 || ackNo3==expAck2 || ackNo3==expAck3 || ackNo3==expAck4){
			recieved_ack[ackNo3%5]=1;
			recievedAcks+=1;
			printf("ack recieved: %d \n", ackNo3);
		}
		else if((ackNo3 != expAck0 || ackNo3!=expAck1 || ackNo3!=expAck2 || ackNo3!=expAck3 || ackNo3!=expAck4) && ackm!=-1){
			printf("duplicate ack: %d \n", ackNo3);
			duplicateAck+=1;
		}

		ackm=0;
		ackm=recvfrom(sfd,&ackNo4,4,0,(struct sockaddr *)&sock_serv,&l);
		if(ackNo4 == expAck0 || ackNo4==expAck1 || ackNo4==expAck2 || ackNo4==expAck3 || ackNo4==expAck4){
			recieved_ack[ackNo4%5]=1;
			recievedAcks+=1;
			printf("ack recieved: %d \n", ackNo4);
		}
		else if((ackNo4 != expAck0 || ackNo4!=expAck1 || ackNo4!=expAck2 || ackNo4!=expAck3 || ackNo4!=expAck4) && ackm!=-1){
			printf("duplicate ack: %d \n", ackNo4);
			duplicateAck+=1;
		}

		//run recieve ack for as many times as duplicate acks have been recieved
		for(int i=0;i<duplicateAck;i++){
			ackm=recvfrom(sfd,&ackNo1,4,0,(struct sockaddr *)&sock_serv,&l);
			if(ackNo1 == expAck0 || ackNo1==expAck1 || ackNo1==expAck2 || ackNo1==expAck3 || ackNo1==expAck4){
				recieved_ack[ackNo1%5]=1;
				recievedAcks+=1;
				printf("ack recieved: %d \n", ackNo1);
			}
			else if((ackNo4 != expAck0 || ackNo4!=expAck1 || ackNo4!=expAck2 || ackNo4!=expAck3 || ackNo4!=expAck4) && ackm!=-1){
			printf("duplicate ack: %d \n", ackNo4);
			duplicateAck+=1;
		}
		}

		//keep on running this until acks for all 5 packets of the window are not recieved
		// resend any packet whose ack is not recieved
		while(recievedAcks !=5){
			if(recieved_ack[0]==0){
				printf("resending packet: %d \n",expAck0);
				m0=sendto(sfd,(struct packet*)&packet0,(n0+4),0,(struct sockaddr*)&sock_serv,l);

			}
			else{
				expAck0=-1;
			}
			if(recieved_ack[1]==0){
				printf("resending packet: %d \n", expAck1);
				m1=sendto(sfd,(struct packet*)&packet1,(n1+4),0,(struct sockaddr*)&sock_serv,l);

			}
			else{
				expAck1=-1;
			}
			if(recieved_ack[2]==0){
				printf("resending packet: %d \n", expAck2);
				m2=sendto(sfd,(struct packet*)&packet2,(n2+4),0,(struct sockaddr*)&sock_serv,l);

			}
			else{
				expAck2=-1;
			}
			if(recieved_ack[3]==0){
				printf("resending packet: %d \n", expAck3);
				m3=sendto(sfd,(struct packet*)&packet3,(n3+4),0,(struct sockaddr*)&sock_serv,l);

			}
			else{
				expAck3=-1;
			}
			if(recieved_ack[4]==0){
				printf("resending packet: %d \n",expAck4);
				m4=sendto(sfd,(struct packet*)&packet4,(n4+4),0,(struct sockaddr*)&sock_serv,l);

			}
			else{
				expAck4=-1;
			}

			//again wait for acks of resent packets
			for(int index=recievedAcks; index<5; index+=1){
				//ackNo0=-1;
				ackm=recvfrom(sfd,&ackNo0,4,0,(struct sockaddr *)&sock_serv,&l);
				if(ackNo0 == expAck0 || ackNo0==expAck1 || ackNo0==expAck2 || ackNo0==expAck3 || ackNo0==expAck4){
					recieved_ack[ackNo0%5]=1;
					recievedAcks+=1;
					printf("ack recieved: %d \n", ackNo0);
					if(ackNo0==expAck0){ expAck0=-1;}
					else if(ackNo0==expAck1){ expAck1=-1;}
					else if(ackNo0==expAck2){ expAck2=-1;}
					else if(ackNo0==expAck3){ expAck3=-1;}
					else if(ackNo0==expAck4){ expAck4=-1;}
				}
			}
		}

		//count the total size sent
		count+=(m0+m1+m2+m3+m4);

		//empty the first packet
		memset(packet0.buf, 0,BUFFERT);

		//read file into first packet. this will be 0 if the file is finished.
        n0=read(fd,packet0.buf,BUFFERT);
       	}

       	//send empty packet to tell reciever that file has been completely sent
	m0=sendto(sfd,(struct packet*)&packet0,0,0,(struct sockaddr*)&sock_serv,l);

	printf("Number of Bytes transferred : %ld\n",count);
	printf("On a total size of: %ld \n",sz);

    //close socket
    close(sfd);
    //close file
    close(fd);
	return EXIT_SUCCESS;
}

int create_client_socket (int port, char* ipaddr){
    int l;
	int sfd;

	//create time structure
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;

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
    if (inet_pton(AF_INET,ipaddr,&sock_serv.sin_addr)==0){
		printf("Invalid IP adress\n");
		return EXIT_FAILURE;
	}

	//change socket settings for time out implimentation
	if (setsockopt(sfd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
    		perror("Error");
	}

    return sfd;
}
