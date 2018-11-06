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
