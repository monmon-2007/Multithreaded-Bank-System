#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#define RESET "\e[0m"
#define BOLD  "\e[1m"
#define KRED  "\x1B[31m"
#define KCYN  "\x1B[36m"

static pthread_t tid;	



void error(char *msg)
{
	perror(msg);
	exit(0);
}









//Client Thread #1 reading user input
void* command_Input(void* sockfd)
{
	int newsockfd = *(int*)sockfd;
	int n;
	
   	printf("---Program Begin---\nopen\nstart\nexit\n--> ");
	char *choice = malloc(400);
	
	while(1)
	{
		fgets(choice, 400, stdin);	
		n = write(newsockfd, choice, strlen(choice));	
		if(n < 0)
			error("ERROR writing to socket\n");

		bzero(choice, strlen(choice));    
		sleep(2);
		printf("-->  ");
		
		
	}
	printf("Client has ended\n");

	pthread_exit(0);
}

void* msg_response(void* sockfd)
{	
	int newsockfd = *(int*)sockfd;	
    char* buffer = malloc(256);
    int n = 0;
    
    while((n = read(newsockfd, buffer, 256))> 0)
    {	
		
		//if(strcmp(buffer, "Account in Use") ==0)
		//{
		//	 printf("\n\n%s%s!!THEFT ALERT!!The Account your trying to access is in use\n%s\n", KRED, BOLD, RESET);	
		//}
	    if(strcmp(buffer, "display") == 0)
		{
				printf("\n%s%sAccess Granted!\n\ncredit\ndebit\nbalance\nfinish\n%s\n", KCYN, BOLD, RESET);
		}
		else if(strcmp(buffer, "exit")== 0)
		{
			printf("GOODBYE\n");
			bzero(buffer, 256);
			pthread_cancel(tid);
		}
		else
		{
			printf("\n%s\n", buffer);
			bzero(buffer, 256);
		}
	}

	if( n == 0)
	{
        printf("\n\n%s%sSERVICE UNAVAILABLE SERVER SHUTDOWN\n%s\n", KRED, BOLD, RESET);		
		pthread_cancel(tid);
	}
	pthread_exit(0);
}

int main(int argc, char *argv[])
{
	
	int sockfd, portno;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	
	system("clear");
	
	if(argc < 3)
	{
		fprintf(stderr, "ERROR usage %s hostname port\n", argv[0]);
		exit(0); 
	}
	
	portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if(sockfd < 0)
		error("ERROR opening socket\n");
		
	server = gethostbyname(argv[1]);
	
	if(server == NULL)
	{
		fprintf(stderr, "ERROR, so such host\n");
		exit(0);
	}
		
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	
	bcopy((char*) server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);
	
				
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	
	pthread_t tid2;
	pthread_attr_t attr2;
	pthread_attr_init(&attr2);
	int con;
	
	while(1)
	{	
		sleep(3);
		
		con = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)); 
		if(con < 0)
			printf("Trying to connect...\n");
		else
		{
			printf("Success! You are now connected to the server\n");
			break;
		}
	}
	
	
	pthread_create(&tid, &attr,command_Input, (void*)&sockfd);
	pthread_create(&tid2, &attr2,msg_response, (void*)&sockfd);
	
	
	pthread_join(tid, NULL);
	
	close(sockfd);  //TERMIBATES CLIENT-CLOSES SOCKET
	return 0;
}
