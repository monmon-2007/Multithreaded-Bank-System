#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include "server.h"
#include "tokenizer.h"
#include <signal.h>
#include <ctype.h>
#define RESET "\e[0m"
#define BOLD  "\e[1m"

static ServerAccounts serverAcct[20]; 
static int initialized = 0;
static int totalAccounts = 0;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
static int safe = 0;
static int numberOfAccounts = 0;
static int HALT = 0; 
static int cut = 0;

void error(char *msg)
{
	perror(msg);
	exit(1);
	
}

int returnCustomer(char* customer)
{
	int index;
	for(index = 0; index < 20; index++)
	{	
		if(((serverAcct[index].isEmpty == 1) &&  (strlen(serverAcct[index].AcctName) == strlen(customer))))
		{
			  int i;
			  for(i = 0; i < strlen(customer);i++)
			  {
				  if( ((serverAcct[index].AcctName[i]) == customer[i]) && (i = (strlen(customer)-1)))
				 {	
						return index;
				  }
			  }
		}		
	}
	return 0;
}

int isFound(char * customer)
{
	int index;
	for(index = 0; index < 20; index++)
	{	
		if(((serverAcct[index].isEmpty == 1) &&  (strlen(serverAcct[index].AcctName) == strlen(customer))))
		{
			if(strcmp(serverAcct[index].AcctName, customer) == 0)
				return 1;
		}		
	}	
	return 0;
}

void bankCustomer(char* customer)
{

	int i;
	for(i = 0; i < 20;i++)
	{
		if(serverAcct[i].isEmpty == 0)
		{	
			serverAcct[i].AcctName = malloc(200);
			strcpy(serverAcct[i].AcctName, customer);
			serverAcct[i].AcctBal = 0.0;
			serverAcct[i].isActive = 0;
			serverAcct[i].isEmpty = 1;
			totalAccounts++;
			break;
		}
	}	
	
}


void customerSearch(char* customer, int newsockfd)
{
	char* output = malloc(256);
	int found = isFound(customer);
	int n;
	int index = returnCustomer(customer);
	cut = index;
	int loc;
	//printf("LANDED HERE");
	fflush(stdout);
		
	loc = pthread_mutex_trylock(&serverAcct[index].lock);	
	if(loc == 0 && found == 1  )
	{
	//	printf("YES WE FOUND IT\n");
	//	fflush(stdout);

		serverAcct[index].isActive = 1;				
		char* customerCommand = malloc(400);
		
		output = "display";
		write(newsockfd, output, 256);
		bzero((char*)&output, sizeof(output));
		while((n = read(newsockfd, customerCommand, 400))>0)
		{	
			//printf("recieved: -> %s", customerCommand);	

			TokenizerT *si = TKCreate(customerCommand);
			int x;
			char* tkn1 = malloc(400); 
			char* tkn2 = malloc(400);
			
			resetFileCounter();
			tkn1 = TKGetNextToken(si);
			//printf("     %d", strcmp(tkn1, "balance"));			
			if((strlen(tkn1) == strlen(customerCommand)-1) && !((strcmp(tkn1, "balance") == 0)  || (strcmp(tkn1, "finish") == 0) )) //AND DOES NOT EQUAL BALANCE OR FINISH
			{
				resetFileCounter();
				bzero((char*)&tkn1, sizeof(tkn1));
				bzero((char*)&tkn2, sizeof(tkn2));
				
				output = "Please Try again\n";
				x = write(newsockfd, output,256);	
				if(x < 0)
					error("ERROR writing to socket");	
			//	sleep(1);
				bzero((char*)&output, sizeof(output));			
				
			}							
			else if(strcmp(tkn1, "credit") == 0 )
			{
				//printf("IM IN HERE credit function\n");
				
				tkn2 = TKGetNextToken(si);
				float ftemp = strtof(tkn2,0);
				char* out = malloc(256);
				int true = 0;
				int i;
				
				for(i = 0; i < strlen(tkn2); i++ )
				{
					if(isalpha(tkn2[i]))
					{
						true = 1;
						break;
					}
				}
				
				if(true == 1)
				{
					
					char* value = malloc(100);
					sprintf(value, "%.2lf", ftemp);
						
					strcpy(out, "ERROR Incorrect ");
					strcat(out,value);
					
					x = write(newsockfd, out,256);	
					if(x < 0)
						error("ERROR writing to socket");	
			
					bzero((char*)&out, sizeof(out));
					free(out);		
				}
				else if(ftemp == 0 || ftemp < (float)0 || strlen(tkn2) == strlen(customer)-1 )
				{	
					   			
					
					//checksless than and null tkn2
					char* value = malloc(100);
					sprintf(value, "%.2lf", ftemp);
						
					strcpy(out, "ERROR Incorrect ");
					strcat(out,value);
					
					x = write(newsockfd, out,256);	
					if(x < 0)
						error("ERROR writing to socket");	
				//	sleep(1);
					bzero((char*)&out, sizeof(out));
					free(out);									
				}
				else
				{
					serverAcct[index].AcctBal += ftemp;
					output = "Thankyou for your deposit";
					x = write(newsockfd, output,256);	
					if(x < 0)
						error("ERROR writing to socket");	
					bzero((char*)&output, sizeof(output));												
				}
				
				resetFileCounter();
				bzero((char*)&tkn1, sizeof(tkn1));
				bzero((char*)&tkn2, sizeof(tkn2));				
				//bzero((char*)&value, sizeof(value));				
			}
			else if(strcmp(tkn1, "debit") == 0 )
			{	
				//printf("INSIDE DEBIT\n");
				tkn2 = TKGetNextToken(si);
				char* value = malloc(100);	
				char* out = malloc(256);	
							
				float ftemp = strtof(tkn2,0);

				//printf("this is the float %lf\n", ftemp);
				
				if(strlen(tkn2) == 0 || ftemp < (float)0 || ftemp > serverAcct[index].AcctBal)
				{	
					//checksless than and null tkn2
					
					//printf("WRONG\n");
					sprintf(value, "%.2lf", ftemp);	
					
					strcpy(out, "Insufficient funds Please Try again ERROR ");
					
					strcat(out, value);
					x = write(newsockfd, out,256);	
					if(x < 0)
						error("ERROR writing to socket");	
					sleep(1);
					bzero((char*)&out, sizeof(out));									
				}
				else
				{					
					//printf("SUBTRACTING FROM ACCOUNT\n");
					serverAcct[index].AcctBal -= ftemp;
                                      	output = "Thankyou for your withdrawal";
					x = write(newsockfd, output,256);	
					if(x < 0)
						error("ERROR writing to socket");	
					bzero((char*)&output, sizeof(output));

				}
				resetFileCounter();
				bzero((char*)&tkn1, sizeof(tkn1));
				bzero((char*)&tkn2, sizeof(tkn2));			
				
			}	
			else if(strcmp(tkn1, "balance") == 0 )
			{
				//printf("I am inside Balance\n");
				resetFileCounter();
				bzero((char*)&tkn1, sizeof(tkn1));
				bzero((char*)&tkn2, sizeof(tkn2));
				
				char* say = malloc(256);
				char* value = malloc(100);
				strcpy(say, "this is Your Balance: ");
				sprintf(value, "%.2lf", serverAcct[index].AcctBal);
				
				strcat(say, value);	
				
				//printf("--%s--", output);
							
				x = write(newsockfd, say,256);	
				if(x < 0)
					error("ERROR writing to socket");	
				//sleep(1);
				bzero((char*)&say, sizeof(say));		
			}
			else if(strcmp(tkn1, "finish") == 0 )
			{
				serverAcct[index].isActive = 0;
				resetFileCounter();
				bzero((char*)&tkn1, sizeof(tkn1));
				bzero((char*)&tkn2, sizeof(tkn2));
							
				output = "Exiting Account\nopen\nstart\nexit";			
				x = write(newsockfd, output,256);	
				if(x < 0)
					error("ERROR writing to socket");	
				//sleep(1);
				bzero((char*)&output, sizeof(output));		
				pthread_mutex_unlock(&serverAcct[index].lock);	
				break;
			}
			else
			{
				resetFileCounter();
				bzero((char*)&tkn1, sizeof(tkn1));
				bzero((char*)&tkn2, sizeof(tkn2));
							
				output = "Please try again\n";			
				x = write(newsockfd, output,256);	
				if(x < 0)
					error("ERROR writing to socket");	
				//sleep(1);
				bzero((char*)&output, sizeof(output));	
			}					
		}
		//free customerCommand
		
		pthread_mutex_unlock(&serverAcct[index].lock);
	}
	else if(found == 0)
	{
		pthread_mutex_unlock(&serverAcct[index].lock);
		output = "--Please Try again\n";
		n = write(newsockfd, output,256);	
		if(n < 0)
			error("ERROR writing to socket");	
		//sleep(1);
		bzero(&output, strlen(output));					
	}
	else////---------------------------------------------------------------------
	{	
		char* sayt = malloc(256);
		int c;
		sayt = "Account in Use";
		c = write(newsockfd, sayt,256);	
		if(c < 0)
			error("ERROR writing to socket");	
		//sleep(1);
		bzero((char*)&sayt, sizeof(sayt));	
	}	
	free(output);	
}

void saveCustomer(char* customer, int newsockfd)
{	
	int n;
	int found = isFound(customer); 
	char* outMsg = malloc(256);
	
	//printf("--%s--", customer);
	//printf("CHECKING!! %d\n", found);
	//fflush(stdout);

	
	if(found == 1)
	{
	//	printf("READY TO CPY\n");
	    outMsg = "Account Name already taken\n";
	    
		n = write(newsockfd,outMsg,256);	
		if(n < 0)
			error("ERROR writing to socket");
				
		sleep(1);
		bzero((char*)&outMsg, sizeof(outMsg));	
		//printf("end\n");
		//fflush(stdout);
	}
	else
	{	
		bankCustomer(customer);						//save account info			
		outMsg = "Success! Account Created\n";
		n = write(newsockfd, outMsg,256);	
		if(n < 0)
			error("ERROR writing to socket");	
		sleep(1);
		bzero((char*)&outMsg, sizeof(outMsg));	
	}
	
}

void* client_service(void* sockfd)
{	
	int newsockfd = *(int*)sockfd;	
	int n;
	char* inputCommand = malloc(400);
	
	while((n = read(newsockfd, inputCommand, 400)) > 0)
	{
		//printf("THIS IS WHAT I GET:	--%s--\n", inputCommand);
		char* output = malloc(256);
		//int n;
		int x;
		
		TokenizerT *tk = TKCreate(inputCommand);
		
		char* token1 = malloc(400); 
		char* token2 = malloc(400);
		
		token1 = TKGetNextToken(tk);

		if(HALT == 0 && strcmp(token1, "open") == 0 )
		{	
			//printf("inside of open\n");
			token2 = TKGetNextToken(tk);
			int length = strlen(token2);

		    if(length == strlen(inputCommand)-1 || length == 0 || length > 101 || numberOfAccounts == 20)
			{	
				if(numberOfAccounts == 20)
				{
					output = "No more room for new accounts\n";
					x = write(newsockfd, output,256);	
					if(x < 0)
						error("ERROR writing to socket");	
					sleep(1);
					bzero((char*)&output, sizeof(output));		
				}	
				else
				{
					output = "Please Try againz\n";
					x = write(newsockfd, output,256);	
					if(x < 0)
						error("ERROR writing to socket");	
					sleep(1);
					bzero((char*)&output, sizeof(output));									
				}
			}
			else
			{		
				if(safe == 1)
				{
					output = "Please Try again\n";
					x = write(newsockfd, output,256);	
					if(x < 0)
						error("ERROR writing to socket");	
					sleep(1);
					bzero((char*)&output, sizeof(output));	
				}
				else
				{
					pthread_mutex_lock(&mutex);
					safe = 1;
					
					
					saveCustomer(token2, newsockfd);
					numberOfAccounts++;
					
					safe = 0;					
					pthread_mutex_unlock(&mutex);	
		         }
	
			}
			resetFileCounter();
			bzero((char*)&token1, sizeof(token1));
			bzero((char*)&token2, sizeof(token2));							
			//bzero((char*)&inputCommand, sizeof(inputCommand));
		}
		else if(HALT == 1)
		{
			output = "Cannot open accounts at this time\n";
			x = write(newsockfd, output,256);	
			if(x < 0)
				error("ERROR writing to socket");	
			//sleep(1);
			bzero((char*)&output, sizeof(output));
		}
		else if(strcmp(token1, "start") == 0)
		{
			//printf("Inside start\n");
			
			token2 = TKGetNextToken(tk);
			int size = strlen(token2);
			
			
			if(size > 101 || size == 0 )
			{				
				output = "Please Try again\n";
				x = write(newsockfd, output,256);	
				if(x < 0)
					error("ERROR writing to socket");	
				sleep(1);
				bzero((char*)&output, sizeof(output));					
			}
			else
			{
				customerSearch(token2, newsockfd);					
			}
			
			resetFileCounter();
			bzero((char*)&token1, sizeof(token1));
			bzero((char*)&token2, sizeof(token2));		
		}
		else if(strcmp(token1, "exit") == 0)
		{
			output = "exit";
			x = write(newsockfd, output,256);	
			if(x < 0)
				error("ERROR writing to socket");	
			sleep(1);
			bzero((char*)&output, sizeof(output));
			printf("Client Disconnected\n");
			break;			
		}		
		else
		{
			output = "Please Try again\n";
			x = write(newsockfd, output,256);	
			if(x < 0)
				error("ERROR writing to socket");	
			//sleep(1);
			bzero((char*)&output, sizeof(output));
				
			resetFileCounter();
			bzero((char*)&token1,sizeof(token1));
			bzero((char*)&token2, sizeof(token2));					
		}
		//bzero((char*)&inputCommand, sizeof(inputCommand));		
	}
	
	

	if(n < 0)
		error("ERROR reading from socket outside of while in Server\n");
	else if( n == 0)
	{
     	printf("Disconnected client\n");
     	serverAcct[cut].isActive = 0;
		//break;
	}	
	pthread_exit(0);

}



void alarm_handler(int signum)
{	
	HALT = 1;	
	printf("AccoutName\tbalance\t\tisActive\t\tisEmpty\n");
	char* active;
	int p;
	for(p = 0; p < 20;p++)
	{
		if(serverAcct[p].isEmpty == 1)
		{	
			if(serverAcct[p].isActive == 1)
			{	
				active = "IN SERVICE";
			    printf("%d %s\t\t%.2lf\t  %s\t  %d\n",p+1, serverAcct[p].AcctName, serverAcct[p].AcctBal,active, serverAcct[p].isEmpty);
			}
			else
			{
				active = "NOT IN SERVICE";
		    	printf("%d %s\t\t%.2lf\t  %s\t  %d\n",p+1, serverAcct[p].AcctName, serverAcct[p].AcctBal,active, serverAcct[p].isEmpty);
			}	
		}
	}		
	HALT= 0;	  	

  alarm(20);   //set a new alarm for 20 second
}

void* server_dump(void* num)
{
	 //set up alarm handler
	signal(SIGALRM, alarm_handler);
	//schedule the first alarm
	alarm(20);

	while(1)
		pause();
	
	
	pthread_exit(0);
}

int main(int argc, char **argv)
{

	system("clear");
	if(!initialized)
	{
		int i;	
		for(i = 0; i < 20;i++)
			serverAcct[i].isEmpty = 0;
		initialized = 1;	
		int num = 20;		
		pthread_t idz;
		pthread_create(&idz, NULL, server_dump,(void*)&num);			
	}

	
	int sockfd, newsockfd;
	
	struct sockaddr_in serv_addr, cli_addr;
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if(sockfd < 0)
		error("ERROR opening socket");
	
	bzero((char *)&serv_addr, sizeof(serv_addr));
	
    //this server wil be listening on this 		specified port number
	int portno = 57777;
	
	serv_addr.sin_family = AF_INET;
	
	serv_addr.sin_port = htons(portno); //converts portno to network byte order ...hostbyte order to a portno in Nbo
	
	serv_addr.sin_addr.s_addr = INADDR_ANY;// gets the IP Address of this machine 
	
	if(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR on binding\n");
		
	//int x = 
	listen(sockfd, 5);
	
	
	//printf("This is: %d\n", x);
	printf("Waiting for clients on port %d.....\n", portno);
	
	socklen_t clilen = sizeof(cli_addr);
	
	
	while(1)
	{
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);	

		if(newsockfd < 0)
			error("ERROR on accept\n");
		else
			printf("\nSuccess! I got a connection from (%s , %d)\n", inet_ntoa(cli_addr.sin_addr),ntohs(cli_addr.sin_port));
			//AFTER GETTING A CONNECTION FROM CLIENT.....
		
		pthread_t id;
		pthread_create(&id, NULL, client_service, (void*)&newsockfd);//SESSION ACCEPTOR THREAD
			
	}
	
	close(sockfd);
	close(newsockfd);
	return 0;
}
