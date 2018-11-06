/*
 ============================================================================
 Name        : test.c
 Author      : Angus Mo
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
	char *result;

	//int memOp;
	//char *openP, *closeP;
	//int match;

	result = (char *)malloc(strlen(input)*sizeof(char));
	const char delim[2] = {',',' '};

	token = strtok(input, delim);
	//memOp = (strcmp(token, "lw") == 0) ? 1: 0;
	//memOp |= (strcmp(token, "sw") == 0) ? 1: 0;
	do{
		strcat(result, token);
		strcat(result, " ");
		token = strtok(NULL, delim);
	}while(token != NULL);

	/*
	if(memOp){
		openP = strchr(result, '(');
		closeP = strrchr(result, ')');
		assert(openP != NULL);

		assert(closeP != NULL);
		match = strcmp(openP,closeP-4);
		assert(!match);

	}
	*/
	return result;

}


int main(int argc, char *argv[]){
	FILE *input;
	input = fopen(argv[1], "r");
	assert(input != NULL);
	char *line;

	while(fgets(line, 64, input) != NULL){
		printf("%s", progScanner(line));
	}

	fclose(input);
	return 0;
}
