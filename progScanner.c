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

enum op {add=0x20, addi=0x8, sub=0x22, mult=0x18, beq=0x4, lw=0x23,
	sw=0x2b, haltsimulation=0xff};

struct inst{
	int opcode;
	unsigned int rs;
	unsigned int rt;
	unsigned int rd;
	int Imm;
};

int parenthesisMatch(char *memField){
	char **parenthesis;
	parenthesis = (char **)malloc(4*sizeof(char *));
	int i, match1,match2;
	for(i = 0; i < 4; i++){
		parenthesis[i] = (char *)malloc(10*sizeof(char));
		if(i % 2 == 0){
			parenthesis[i] = memField;
		} else{
			parenthesis[i] = (memField + strlen(memField));
		}
	}

	do{
		if(*parenthesis[0] != '('){
			parenthesis[0]++;
		}
		if(*parenthesis[1] != '('){
			parenthesis[1]--;
		} else{
			break;
		}
	}while(parenthesis[1] >= parenthesis[0]);

	match1 = parenthesis[1]-parenthesis[0];

	do{
		if(*parenthesis[3] != ')'){
			parenthesis[3]--;
		}
		if(*parenthesis[2] != ')'){
			parenthesis[2]++;
		} else{
			break;
		}
	}while(parenthesis[3] >= parenthesis[2]);

	match2 = parenthesis[3]-parenthesis[2];

	free(parenthesis);
	return (match1 == 0) && (match2 == 0);
}

char *progScanner(char *inputLine){
	char **tokens;
	char *retLine;
	retLine = (char *)malloc(256*sizeof(char));
	tokens = (char **)malloc(10*sizeof(char *));
	char delims[] = {',',' '};
	int i,j,k;
	int memOp;
	for(i = 0; i < 10; i++){
		tokens[i] = (char *)malloc(256*sizeof(char));
	}

	i = 0;
	tokens[i] = strtok(inputLine, delims);
	memOp = (strcmp(tokens[i], "lw") == 0) ? 1 : 0;
	memOp |= (strcmp(tokens[i], "sw") == 0) ? 1 : 0;
	while(tokens[i] != NULL){
		tokens[++i] = strtok(NULL, delims);
	}

	i = 0;
	k = 0;
	while(tokens[i] != NULL){
		j = 0;
		if(memOp && i == 2){
			if(!parenthesisMatch(tokens[i])){
				return NULL;
			}
			char c;
			while(j < (int)strlen(tokens[i])){
				c = *(tokens[i]+j);
				retLine[k++] = (c == '(' || c == ')')? ' ': c;
				j++;
			}
			i++;
			continue;
		}
		while(j < (int)strlen(tokens[i])){
			retLine[k++] = *(tokens[i]+j);
			j++;
		}
		retLine[k++] = ' ';
		i++;
	}
	retLine[k] = '\0';
	free(tokens);
	return retLine;
}

struct inst parser(char *instStr)
{
	struct inst instruction;
	char *opField;
	char *regFields[3];
	char *Imm;
	char *memField;

	opField = strtok(instStr, " ");


	switch(opField[0]){
	case 'a':
		if(strcmp(opField+1, "dd") == 0){
			instruction.opcode = add;
			break;
		} else if(strcmp(opField+1, "ddi") == 0){
			instruction.opcode = addi;
			break;
		}
	case 's':
		if(strcmp(opField+1, "ub") == 0){
			instruction.opcode = sub;
			break;
		} else if(strcmp(opField+1, "w") == 0){
			instruction.opcode = sw;
			break;
		}
	case 'b':
		if(strcmp(opField+1, "eq") == 0){
			instruction.opcode = beq;
			break;
		}
	case 'l':
		if(strcmp(opField+1, "w") == 0){
			instruction.opcode = lw;
		} else{
			instruction.opcode = -1;
			return instruction;
		}
		break;
	case 'm':
		if(strcmp(opField+1, "ult") == 0){
			instruction.opcode = mult;
			break;
		}
	case 'h':
		if(strcmp(opField+1, "altsimulation") == 0){
			instruction.opcode = haltsimulation;
			return instruction;
		}
	default:
		instruction.opcode = -1;
		return instruction;
		break;
	}

	regFields[0] = strtok(NULL, " ");

	switch(instruction.opcode){
	case lw:
	case sw:
		Imm = strtok(NULL, " ");
		regFields[1] = strtok(NULL, " ");
		instruction.rt = regNumberConverter(regFields[0]);
		instruction.rs = regNumberConverter(regFields[1]);
		instruction.Imm = (int)atoi(Imm);
		break;
	case beq:
		regFields[1] = strtok(NULL, " ");
		Imm = strtok(NULL, " ");
		instruction.rs = regNumberConverter(regFields[0]);
		instruction.rt = regNumberConverter(regFields[1]);
		instruction.Imm = (int)atoi(Imm);
		break;
	case addi:
		regFields[1] = strtok(NULL, " ");
		Imm = strtok(NULL, " ");
		instruction.rt = regNumberConverter(regFields[0]);
		instruction.rs = regNumberConverter(regFields[1]);
		instruction.Imm = (int)atoi(Imm);
		break;
	case mult:
		regFields[1] = strtok(NULL, " ");
		instruction.rs = regNumberConverter(regFields[0]);
		instruction.rt = regNumberConverter(regFields[1]);
		break;
	case add:
	case sub:
		regFields[1] = strtok(NULL, " ");
		regFields[2] = strtok(NULL, " ");
		instruction.rd = regNumberConverter(regFields[0])
		instruction.rs = regNumberConverter(regFields[1]);
		instruction.rt = regNumberConverter(regFields[2]);
		break;
	default:
		break;
	}

	return instruction;
}

int regNumberConverter(char *reg)
{
	int regNum = -1;
	char *token;
	char *delims = " $";
	token = strtok(reg, delims);
	printf("\n%s\n", reg);
	puts(token);

	if(strlen(token) > 2 && strcmp(token, "zero") || reg[0] != '$')
	{
		puts("Incorrect register syntax");
		exit(0);
	}

	if((atoi(token) <= 25 && atoi(token) >= 10) || !strcmp(token, "0") || !strcmp(token, "8") || !strcmp(token, "9"))
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

int main(int argc, char *argv[])
{
	FILE *input;
	input = fopen(argv[1], "r");
	assert(input != NULL);
	char *line;
	char *fmtLine;
	line = (char *)malloc(64*sizeof(char));
	struct inst temp;

	unsigned int lineNum = 1;
	while(fgets(line, 64, input)){
		fmtLine = (char *)malloc(64*sizeof(char));
		fmtLine = progScanner(strdup(line));
		printf("%s\n", fmtLine);
		free(fmtLine);

		temp = parser(progScanner(fmtLine));

		lineNum++;

	}

	free(line);
	free(fmtLine);
	fclose(input);
	return 0;
}
