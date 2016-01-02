#ifndef TOKENIZER_H_
#define TOKENIZER_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct TokenizerT_ 
{	
	char *token;		//ouput
	char *strArray;

};
typedef struct TokenizerT_ TokenizerT;

void resetFileCounter();

TokenizerT *TKCreate(char *ts );

void TKDestroy( TokenizerT * tk );

char *TKGetNextToken( TokenizerT * tk );

#endif

