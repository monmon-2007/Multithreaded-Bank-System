#include <stdio.h>
#include <stdlib.h>

typedef struct ServerAccounts
{
	char *AcctName;
	float AcctBal;
	int isActive;
	int isEmpty;
	pthread_mutex_t lock;
}ServerAccounts;
