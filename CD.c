#include<stdio.h> 
#include<string.h>
#include<ctype.h>
#include<stdlib.h>

#define KEYWORD  "Keyword"
#define ID 	 "Identifier"
#define NLITERAL "Number"
#define SLITERAL "String"
#define SS 	 "Symbol"
#define ROP 	 "RelationalOp"
#define FLOOP_S "for_loop_start"
#define FLOOP_E "for_loop_end"
#define SEMICOLON ';'
#define RIGHTBRACKET ')'
#define LEFTBRACE '{'
#define RIGHTBRACE '}'
#define KEYLEN 13
#define SYMLEN 24
#define SYMLEN2 4

char keywords[][10] = {"while", "if", "do", "for", "int", "char", "float", "double", "include", "switch", "case", "return", "main"};
char symbols[] = {'<', '>', '=', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '-', '=', '+', '{', '}', '.', '[', ']', ';', ',', '/'};
char symbols2[][3] = {"<=", ">=", "==", "!="};
char declarations[20][15];
int declarationCounter = 0;
int loopNumber = 0;
int ifNumber = 0;
int numOfBrace;
int globalLine;
int numOfError;

struct token
{
    	char name[20];
    	char tokenType[20];
    	char strVal[20];
    	int numVal;
    	int lineNum;
    	struct token *lPtr;
    	struct token *rPtr;
} *ptr;
struct token *current, *start = NULL, *temp;

void scanner(); 
void parser(); 
int mygetc(FILE *);
int isNum(char); 
int isOp(char); 
int isOperator(int); 
int isRelOperatorType1(int); 
int isAlreadyDeclared();
struct token* createList(struct token *, char *, char *, int, char *);  
void getToken();
int isMatching(char *,int); 
int isMatchingType(char *, int);
int checkExp(int, int); 
int checkExpWithoutEQUALTO(int);
int checkRelExp(int); 
int checkOpeningBrace(int); 
int checkClosingBrace(int); 
int checkFor(int);
void printError();
int isIdentifierWithArraySupport(int, int);

void scanner()
{
	start = createList(start, "NAME", "TOKENTYPE", 0, "STRINGVAL");
    	current = start;
    	int k, i, num;
    	char c, need;
    	int gotKeyword = 0; //Flag to set if keyword found or not
	int got2SymOp; 	//Flag for Relation Operators
	char buffer[100];
    	globalLine = 1;

	FILE *fileToBeOpened;
	
    	fileToBeOpened = fopen("testFile.c","r"); 

    	do
    	{
        	c = mygetc(fileToBeOpened);

		//String Detection
        	if(c == '"')
        	{
            		k=0;
            		c = mygetc(fileToBeOpened); //Get next character and increment if character = newline
            		for( ; ; )
            		{
                		if(c == '"')
                    			break;
                		buffer[k++] = c;
                		c = mygetc(fileToBeOpened);
            		}
            		buffer[k] = '\0';
            		current = createList(current," ",SLITERAL,0,buffer);
        	}
        	
		//Keyword and Identifier Detection
		if(isalpha(c))
        	{
            		k = 0;
            		gotKeyword = 0;
            		buffer[k++] = c;
            		for( ; ; )
            		{
                		c = mygetc(fileToBeOpened);
                		if(!isalpha(c))
                    			break;
                		buffer[k++] = c;
            		}
            		buffer[k]='\0';
            		for(i = 0; i < KEYLEN; i++)
            		{
                		if(!strcmp(keywords[i],buffer))
                		{
					//Save as keyword in token linked list
                    			current = createList(current,buffer,KEYWORD,0," ");
                    			gotKeyword = 1;
                    			break;
                		}
            		}
			//Save as identifier in token linked list
            		if(!gotKeyword)
            			current = createList(current,buffer,ID,0," ");
        	}
        
		//Numerals Detection
        	num = 0;
        	if(isNum(c))
        	{
            		num = c - '0';
            		for( ; ; )
            		{
                		c = mygetc(fileToBeOpened);
                		if(!isNum(c))
                    			break;
                		num *= 10;
                		num = num + (c -'0');
            		}
            		current = createList(current," ",NLITERAL,num," ");
        	}
		
		//Operators 
               	if(isOp(c))
        	{
            		k = 0;
            		got2SymOp = 0;
            		buffer[k++] = c;
            		c = mygetc(fileToBeOpened);
            		if(isOp(c))
            		{
                		buffer[k++] = c;
                		buffer[k] = '\0';
                		for(i = 0; i < SYMLEN2; i++)
                			if(!strcmp(symbols2[i],buffer))
                    			{
                        			current = createList(current,buffer,ROP,0," ");
                        			got2SymOp = 1;
                        			break;
                    			}
                	}
            		else
            		{
                		if(c == '\n')
                    			globalLine--;
                		fseek(fileToBeOpened,-1,SEEK_CUR);
            		}

            		if(!got2SymOp)
            		{
		                need = buffer[1];
                		buffer[1] = '\0';
                		current = createList(current,buffer,SS,0," ");
                		if(k != 1)
                		{
                    			buffer[0] = need;
                    			buffer[1] = '\0';
                    			current = createList(current,buffer,SS,0," ");
                		}
            		}
        	}
    	}while(c != EOF);
	printf("\n \n \t \t SYMBOL TABLE\n\n");
	printf("\nToken \t \t Type \t \t Value  \t Line");
    	for(temp = start->rPtr; temp!=NULL; temp = temp->rPtr)
		printf("\n%s \t \t %s \t %d \t \t %d", temp->name, temp->tokenType, temp->numVal, temp->lineNum);
	printf("\n\n\n\n");
}

void parser()
{
	ptr  = start;
    	numOfError = 0;
	freopen("ErrorFile.txt", "w", stdout);
	
	printf("\t \t ERRORS\n");
    	while(isMatching("#", 1))
    	{
        	if(!isMatching("include", 1))	
            		printError();
		
        	if(!isMatching("<", 1))		
            		printError();
	
        	getToken();   			
        	getToken();   			
        	getToken();   			
		if(!isMatching(">", 1))		
            		printError();
    	}
		
        if(!isMatching("int", 0) && !isMatching("void", 0))
        	printError();
    	if(!isMatching("main", 1))
        	printError();
	
    	if(!isMatching("(", 1))
        	printError();
    	if(!isMatching(")", 1))
        	printError();

	if(!checkOpeningBrace(1))
		printError();


    	while(isMatching("int", 1) || isMatching("char", 0))
    	{
		while(isIdentifierWithArraySupport(1, 1))
        	{
            		if(!isMatching(",", 1)  && !isMatching(";", 0))
            			printError();
			if(isMatching(";", 0))
			{
				
            			break;
			}
			else
			{
				
			}				
        	}
    	}

    	temp = ptr;
    	ptr = ptr->lPtr;
    	while(ptr->rPtr != NULL)
    	{
        	if(checkFor(1)) 
		{
					
		}
        	else if(checkClosingBrace(0)) 
		{
			
		}
        	
        	else if(checkExp(0, 1)) 
		{
		
		}
        	else printError();
    	}
    
	if(numOfBrace)
        	printError();
    	printf("\n%d is the number of errors",numOfError);
}

int mygetc(FILE *stream)
{
    	int x=getc(stream);
    	if(x == '\n')
        	globalLine++;
    	return x;
}

int isNum(char c)
{
    	if(c >= '0' && c <= '9')
        	return 1;
    	return 0;
}

int isOp(char c)   
{
    	int i;
    	for(i = 0; i < SYMLEN; i++)
    	if(!(c-symbols[i]))
        	return 1;
    	return 0;
}

int isOperator(int lookForNextToken)  
{
	if(lookForNextToken)
		getToken();

	switch(ptr->name[0])
	{
		case '+':
		case '-':
		case '%':
		case '/':
		case '*':return 1;
	}
	return 0;
}

int isRelOperatorType1(int lookForNextToken) 
{
	if(lookForNextToken)
		getToken();

	switch(ptr->name[0])
	{
		case '<':
		case '>':return 1;
	}
	return 0;
}

int isAlreadyDeclared()
{
    	int i;
    	for(i = 0; i < declarationCounter; i++)
        	if(isMatching(declarations[i], 0))
            		return 1;
    	return 0;
}

struct token* createList(struct token *crt, char *name, char *tokenType, int num, char *str)
{
    	struct token *tmp = (struct token *) malloc(sizeof(struct token));
    	strcpy(tmp->name,name);
    	strcpy(tmp->tokenType,tokenType);
    	strcpy(tmp->strVal,str);
    	tmp->lineNum = globalLine;
    	tmp->numVal = num;
    	tmp->rPtr = NULL;
    	if(crt == NULL)
        	return tmp;
    	tmp->lPtr = crt;
    	crt->rPtr = tmp;
	
	return tmp;
}

void getToken()
{
	ptr = ptr->rPtr;
}

int isMatching(char *str,int lookForNextToken)            
{
	if(lookForNextToken)
		getToken();

	if(strcmp(str,ptr->name) == 0)
		return 1;
	return 0;
}

int isMatchingType(char *str,int lookForNextToken)
{
	if(lookForNextToken)
		getToken();

	if(strcmp(str,ptr->tokenType) == 0)
		return 1;
	return 0;
}

int checkExp(int lookForNextToken, int matchSemiColon)
{
	if(!isIdentifierWithArraySupport(lookForNextToken, 0))
		return 0;
	if(!isMatching("=", 1))
		return 0;
	for( ; ; )
	{
		if(!isIdentifierWithArraySupport(1, 0) && !isMatchingType(NLITERAL, 0))
			return 0;
		if(!isOperator(1))
			break;
	}
	if(matchSemiColon)
		if(!isMatching(";", 0))
		{
			ptr = ptr->lPtr;
			return 0;
	    	}
	return 1;
}

int checkExpWithoutEQUALTO(int lookForNextToken)
{
    	for( ; ; )
	{
		if(!isIdentifierWithArraySupport(lookForNextToken,0) && !isMatchingType(NLITERAL,0))
			return 0;
		if(!isOperator(1))
			break;
		lookForNextToken = 1;
	}
	return 1;
}

int checkRelExp(int lookForNextToken)
{
	if(!checkExpWithoutEQUALTO(lookForNextToken))
		return 0;
	if(!isMatchingType(ROP, 0) && !isRelOperatorType1(0))
		return 0;
	if(!checkExpWithoutEQUALTO(1))
		return 0;
	return 1;
}

int checkOpeningBrace(int lookForNextToken)
{
   	if(!isMatching("{",lookForNextToken))
		return 0;
    	numOfBrace++;
    	return  1;
}

int checkClosingBrace(int lookForNextToken)
{
    	if(numOfBrace == 0)
    	{
		printError();
		return 0;
    	}
    	if(!isMatching("}",lookForNextToken))
		return 0;
    	numOfBrace--;
    	return  1;
}

int checkFor(int lookForNextToken)
{
	if(!isMatching("for", lookForNextToken))
		return 0;
	if(!isMatching("(", 1))
		return 0;
	if(!checkExp(1, 1))
		return 0;
	if(!checkRelExp(1))
		return 0;
	if(!isMatching(";", 0))
		return 0;
	if(!checkExp(1, 0))
		return 0;
	if(!isMatching(")", 0))
		return 0;
	if(!checkOpeningBrace(1))
		return 0;
    	return 1;
}

void printError() 
{
	numOfError++;
	printf("\nLine %d: %s",ptr->lineNum,ptr->name);
}

int isIdentifierWithArraySupport(int lookForNextToken,int declareIdentifierPlz)
{
    	int i;
    	if(!isMatchingType(ID,lookForNextToken))
        	return 0;
    	if(declareIdentifierPlz)
	    	strcpy(declarations[declarationCounter++],ptr->name);
    	if(!isAlreadyDeclared())
    	{
        	return 0;
    	}
    	if(!isMatching("[",1))
    	{
        	ptr = ptr->lPtr;
        	return 1;
    	}
    	if(!isMatchingType(NLITERAL,1))
        	return 0;
    	if(!isMatching("]",1))
        	return 0;
    	return 1;
}

