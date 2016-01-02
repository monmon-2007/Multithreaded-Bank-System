#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokenizer.h"
#include <ctype.h>



TokenizerT *TKCreate(char *ts ) 
{
	TokenizerT *tokenizer = malloc(sizeof(struct TokenizerT_));	
	
	tokenizer->strArray = malloc(sizeof(char)*strlen(ts)+1);
	
	tokenizer->token = malloc(sizeof(char)*strlen(ts)+1);
	
	strcpy(tokenizer->strArray,ts);

  return tokenizer;
}



void TKDestroy( TokenizerT * tk ) 
{
	free (tk->token);
	free (tk->strArray);

}

static int counter = 0; 
void resetFileCounter()
{
	counter = 0;
}
char *TKGetNextToken( TokenizerT * tk ) 
{

	while(tk->strArray[counter] == ' ' || tk->strArray[counter] == '\t'  ) //skips initial white spaces(ispunct(tk->strArray[counter]))
	{	
		counter++;
	}
	
	int tokenCount = 0;
	while(   (tk->strArray[counter] != ' ') && (tk->strArray[counter] != '\0') && tk->strArray[counter] != '\t' )//checks seperator and gets 1 token  && (!ispunct(tk->strArray[counter]))
	{
		
		tk->token[tokenCount] = tk->strArray[counter];
		counter++;
		tokenCount++;

	}
	tk->token[strcspn(tk->token, "\n")] = '\0';
	char* mine = tk->token;
	const int l = strlen(mine);
	char* low = (char*)malloc(l+1);
	low[l] = 0;
	int i;
	for(i = 0; i < l; i++ )
	{
		low[ i ] = tolower( mine[ i ] );
	}		
	tk->token = low;
		
	char* temp = malloc(400);
	strcpy(temp,tk->token);
	return temp;
 
}
