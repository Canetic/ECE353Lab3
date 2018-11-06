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

char *progScanner(char *input){

	char *token;
	char *retString;

	//int memOp;
	//char *openP, *closeP;
	//int match;

	retString = (char *)malloc(strlen(input)*sizeof(char));
	const char delim[2] = {',',' '};

	token = strtok(input, delim);
	strcpy(retString, token);
	token = strtok(NULL, delim);
	//memOp = (strcmp(token, "lw") == 0) ? 1: 0;
	//memOp |= (strcmp(token, "sw") == 0) ? 1: 0;
	do{
		strcat(retString, " ");
		strcat(retString, token);
		token = strtok(NULL, delim);
	}while(token != NULL);

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
	return retString;

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
