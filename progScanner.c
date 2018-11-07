/*
 ============================================================================
 Name        : test.c
 Author      :
 Version     : 0.6
 Copyright   : Your copyright notice
 Description :
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>


struct inst{
	int opcode;
	int rs;
	int rd;
	int Imm;
};

char *progScanner(char *line){
	char *token;
	char *retString;

	//set memory for the return string
	retString = (char *)malloc(strlen(line)*sizeof(char));
	//declare the delimiters
	const char delim[2] = {',',' '};

	//copy the opcode into the return string
	token = strtok(line, delim);
	strcpy(retString, token);

	//find the next parameter
	token = strtok(NULL, delim);
	while(token != NULL){
		//continue adding parameters separated by a single space
		strcat(retString, " ");
		strcat(retString, token);
		token = strtok(NULL, delim);
	}

	//return the resulting formatted string
	return retString;
}

struct inst parser(char *instStr){
	struct inst instruction;
	//int memOp;
		//char *openP, *closeP;
		//int match;
	//memOp = (strcmp(token, "lw") == 0) ? 1: 0;
		//memOp |= (strcmp(token, "sw") == 0) ? 1: 0;
	/*
		if(memOp){
			openP = strchr(retString, '(');
			closeP = strrchr(retString, ')');
			assert(openP != NULL);

			assert(closeP != NULL);
			match = strcmp(openP,closeP-4);
			assert(!match);

		}
		*/
}

int regNumberConverter(char *reg)
{
	int regNum = -1;
	char *token;
	char *delims = " $";
	token = strtok(reg, delims);
	printf("\n%s\n", reg);
	puts(token);

	if(strlen(token) > 2 && strcmp(token, "zero"))
	{
		puts("Incorrect register syntax");
		exit(0);
	}

	if((atoi(token) <= 25 && atoi(token) >= 10) || !strcmp(token, "8") || !strcmp(token, "9"))
	{
		regNum = atoi(token);
	} else {
		switch(token[0])
		{
		case 's':
			switch(token[1])
			{
			case '0':
				regNum = 16;
				break;
			case '1':
				regNum = 17;
				break;
			case '2':
				regNum = 18;
				break;
			case '3':
				regNum = 19;
				break;
			case '4':
				regNum = 20;
				break;
			case '5':
				regNum = 21;
				break;
			case '6':
				regNum = 22;
				break;
			case '7':
				regNum = 23;
				break;
			default:
				break;
			}
			break;
		case 't':
			switch(token[1])
			{
			case '0':
				regNum = 8;
				break;
			case '1':
				regNum = 9;
				break;
			case '2':
				regNum = 10;
				break;
			case '3':
				regNum = 11;
				break;
			case '4':
				regNum = 12;
				break;
			case '5':
				regNum = 13;
				break;
			case '6':
				regNum = 14;
				break;
			case '7':
				regNum = 15;
				break;
			case '8':
				regNum = 24;
				break;
			case '9':
				regNum = 25;
				break;
			default:
				break;
			}
			break;
		case 'z':
			if(!strcmp(token, "zero"))
				regNum = 0;
			break;
		default:
			break;
		}
	}

	if(regNum == -1)
	{
		puts("Incorrect register syntax");
		exit(0);
	}
	return regNum;
}

int main(int argc, char *argv[]){
	FILE *input;
	input = fopen(argv[1], "r");
	assert(input != NULL);
	char *line;
	line = (char *)malloc(64*sizeof(char));

	while(fgets(line, 64, input)){
		printf("%s", progScanner(line));
	}

	free(line);
	fclose(input);
	return 0;
}
