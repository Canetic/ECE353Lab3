/*
 ============================================================================
 Name        : progScan.c
 Author      :
 Version     : 11.14
 Description :
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

void MEM();
void WB();
void EX();
void IF();
void ID();

//storing registers and data memory
int dataMemory[512];
//program counter
int pc =0;
int registers[32];
//intializing register 0 as 0
//registers[0] = 0;
//define a set of opcodes
enum op {add=0x20, addi=0x8, sub=0x22, mult=0x18, beq=0x4, lw=0x23,
	sw=0x2b, haltsimulation=0xff};
int errorCode = 0;	//variable holding the error code during program execution

// instruction 
struct inst{
	int opcode;
	unsigned int rs;
	unsigned int rt;
	unsigned int rd;
	int Imm;
	int result;
};

//latche struct 
struct latch{
	
	struct inst instruction;
	int read;
	int write;
	
};

//latches
  struct latch IFIDLatch;
  struct latch IDEXLatch;
  struct latch EXMEMLatch;
  struct latch MEMWBLatch;
  
//Function to determine and validate the immediate field//
int immediateParse(char *immediate){
	char *ptr = immediate;
	int isValid = 1;
	int Imm;
	//test if the string is NULL
	if(immediate == NULL){
		return 0;
		//check for '+' and '-' signs
	} else if(strdup(strtok(immediate,"+-")) != NULL){
		ptr = strtok(immediate,"+-");
	}

	//test each character to determine if the immediate is a numerical integer
	while(*ptr != '\0'){
		isValid &= !(isalpha(*ptr)) && (*ptr != '.');
		ptr++;
	}
	if(isValid){
		Imm = (int)atoi(immediate);
		//determine if the immediate is out of bounds
		if(abs(Imm) < 0x10000){
			return Imm;
		}
	}
	errorCode = 'i';
	return 0;
}

//Function to test whether the memory operation field is in the correct format//
int parenthesisMatch(char *memField){
	//setup 2 test cases, 2 for each parentheses type
	char **parenthesis;
	parenthesis = (char **)malloc(4*sizeof(char *));
	int i, match1,match2;
	//mathc variables
	int p,p1,p2;
	for(i = 0; i < 4; i++){
		parenthesis[i] = (char *)malloc(10*sizeof(char));
		if(i % 2 == 0){
			parenthesis[i] = memField;
		} else{
			parenthesis[i] = (memField + strlen(memField));
		}
	}

	p1 = 0;
	p2 = 0;
	//determine if the open parentheses match, if any
	do{
		//search from the left
		if(*parenthesis[0] != '('){
			parenthesis[0]++;
		} else{
			p1 = 1;
		}
		//search from the right
		if(*parenthesis[1] != '('){
			parenthesis[1]--;
		} else{
			p2 = 1;
		}
		p = p1&p2;
		//stop when the pointers cross or when both parentheses are found
	}while((parenthesis[1] > parenthesis[0]) && !p);
	//determine if the parentheses match, this should be 0
	match1 = parenthesis[1]-parenthesis[0];

	p1 = 0;
	p2 = 0;
	//determine if the close parentheses match, if any
	do{
		//search from the right
		if(*parenthesis[3] != ')'){
			parenthesis[3]--;
		} else{
			p1 = 1;
		}
		//search from the left
		if(*parenthesis[2] != ')'){
			parenthesis[2]++;
		} else{
			p2 = 1;
		}
		//stop when the pointers cross or when both parentheses are found
	}while((parenthesis[3] > parenthesis[2]) && !p);
	//determine if the parentheses match, this should be 0
	match2 = parenthesis[3]-parenthesis[2];

	free(parenthesis);
	//return if both match
	return (match1 == 0) && (match2 == 0);
}

char *progScanner(char *inputLine){
	char **tokens;	//declare the parameter array
	char *retLine;	//declare the formatted string variable
	//declare the delimeters
	retLine = (char *)malloc(100*sizeof(char));
	tokens = (char **)malloc(10*sizeof(char *));
	char delims[] = {',',' ','\r','\n'};
	int i,j,k;
	int memOp;
	for(i = 0; i < 10; i++){
		tokens[i] = (char *)malloc(256*sizeof(char));
	}

	//determine the first parameter, this should be the opcode
	i = 0;
	tokens[i] = strtok(inputLine, delims);
	//return null character if the input instruction is whitespace
	if(!tokens[i] || !strcmp(tokens[i],"")){
		*retLine = '\0';
		return retLine;
	}
	//determine whether or not this is a memory operation
	memOp = (strcmp(tokens[i], "lw") == 0) ? 1 : 0;
	memOp |= (strcmp(tokens[i], "sw") == 0) ? 1 : 0;

	//determine the rest of the parameters
	while(tokens[i] != NULL){
		tokens[++i] = strtok(NULL, delims);
	}
	//if there are more than 4 parameters, the format is incorrect
	if(i > 4){
		errorCode = 'f';
		return "";
	}

	i = 0;
	k = 0;
	//concatenate the formatted string
	while(tokens[i] != NULL){
		j = 0;
		//perform if this is a memory operation
		if(memOp && i == 2){
			//determine if the parentheses match first
			if(!parenthesisMatch(tokens[i])){
				errorCode = 'p';
				return "";
			}
			//replace all parentheses with a space
			char c;
			while(j < (int)strlen(tokens[i])){
				c = *(tokens[i]+j);
				retLine[k++] = (c == '(' || c == ')')? ' ': c;
				j++;
			}
			i++;
			continue;
		}
		//append the parameters to the formatted string
		while(j < (int)strlen(tokens[i])){
			retLine[k++] = *(tokens[i]+j);
			j++;
		}
		//separate each field by a single space
		retLine[k++] = ' ';
		i++;
	}
	//add a null character at the end to validate the string
	retLine[k] = '\0';
	free(tokens);
	return retLine;
}

int regNumberConverter(char *reg)
{
	int regNum = -1;
	char *token;
	char *delims = " $";
	token = strtok(reg, delims);

	//test whether the string passed in is NULL
	if(reg == NULL){
		errorCode = 'f';
		return regNum;
		//determine if the register field is the correct format
	} else if((strlen(token) > 2) && (strcmp(token, "zero")) || (reg[0] != '$'))
	{
		errorCode = 'r';
		return regNum;
	}
	//determine the register value if it is numerical
	if((atoi(token) <= 25 && atoi(token) >= 10) || !strcmp(token, "0") || !strcmp(token, "8") || !strcmp(token, "9"))
	{
		regNum = atoi(token);
	} else {
		//determine the register value if it is symbolic
		switch(token[0])
		{
		//save registers
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
			//temporary registers
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
				//if the register is zero
				case 'z':
					if(!strcmp(token, "zero"))
						regNum = 0;
					break;
				default:
					break;
		}
	}

	//if none of the cases match, note the error
	if(regNum == -1)
	{
		errorCode = 'r';
	}
	return regNum;
}

struct inst parser(char *instStr)
{
	struct inst instruction;
	char *opField;
	char *regFields[3];
	char *Imm;

	//the string passed in cannot be NULL
	assert(instStr != NULL);
	//determine the opcode
	opField = strtok(instStr, " ");
	assert(opField != NULL);
	//determine if the opcode is valid
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
			break;
		}
	case 'm':
		if(strcmp(opField+1, "ult") == 0){
			instruction.opcode = mult;
			break;
		}
	case 'h':
		//case for haltsimulation command, test if there are any other strings following it
		if((strcmp(opField+1, "altsimulation") == 0) && (strtok(NULL, " ") == NULL)){
			instruction.opcode = haltsimulation;
			return instruction;
		}
	default:
		//if the opcode is invalid note the error
		errorCode = 'o';
		return instruction;
		break;
	}

	//determine the first register value
	regFields[0] = strtok(NULL, " ");
	assert(regFields[0] != NULL);
	//determine the last fields based on the opcode
	switch(instruction.opcode){
	case lw:
	case sw:
		Imm = strtok(NULL, " ");
		regFields[1] = strtok(NULL, " ");
		instruction.rt = regNumberConverter(regFields[0]);
		instruction.rs = regNumberConverter(regFields[1]);
		instruction.Imm = immediateParse(Imm);
		break;
	case beq:
		regFields[1] = strtok(NULL, " ");
		Imm = strtok(NULL, " ");
		instruction.rs = regNumberConverter(regFields[0]);
		instruction.rt = regNumberConverter(regFields[1]);
		instruction.Imm = immediateParse(Imm);
		break;
	case addi:
		regFields[1] = strtok(NULL, " ");
		Imm = strtok(NULL, " ");
		instruction.rt = regNumberConverter(regFields[0]);
		instruction.rs = regNumberConverter(regFields[1]);
		instruction.Imm = immediateParse(Imm);
		break;
	case mult:
		//		regFields[1] = strtok(NULL, " ");
		//		instruction.rs = regNumberConverter(regFields[0]);
		//		instruction.rt = regNumberConverter(regFields[1]);
		//		break;
	case add:
	case sub:
		regFields[1] = strtok(NULL, " ");
		regFields[2] = strtok(NULL, " ");
		instruction.rd = regNumberConverter(regFields[0]);
		instruction.rs = regNumberConverter(regFields[1]);
		instruction.rt = regNumberConverter(regFields[2]);
		break;
	default:
		break;
	}

	return instruction;
}

//Function to compile the input file and store into the instruction memory//
void fileParser(FILE *fp, char *fileName){
	char *line;
	char *fmtLine;
	line = (char *)malloc(100*sizeof(char));
	struct inst instruction;
	int lineNum, instrAddr;

	lineNum = 1;
	instrAddr = 0;
	while(fgets(line, 100, fp)){
		fmtLine = (char *)malloc(100*sizeof(char));
		//format the input instruction
		fmtLine = progScanner(strdup(line));
		//continue only if the formatted string isn't NULL
		assert(fmtLine != NULL);

		//load valid instructions into the instruction memory if it isn't full
		if((strcmp(fmtLine, "")!=0) && (instrAddr < 512)){
			instruction = parser(fmtLine);
			instrAddr++;
		}

		//check for errors before continuing
		if(errorCode != 0){
			printf("%s:%d \"%s\"\nerror: ", fileName, lineNum, strtok(line,"\r\n"));
			switch(errorCode){
			case 'f':
				puts("Incorrect instruction format");
				exit(0);
				break;
			case 'i':
				puts("Immediate must be and integer between -65,535 and 65,534");
				exit(0);
				break;
			case 'o':
				puts("Invalid opcode");
				exit(0);
				break;
			case 'p':
				puts("Mismatched parentheses");
				exit(0);
				break;
			case 'r':
				puts("Incorrect register syntax");
				exit(0);
				break;
			default:
				break;
			}
		}
		free(fmtLine);
		lineNum++;
	}

	free(line);
	return;
}

int main(int argc, char *argv[])
{
	FILE *input;
	input = fopen(argv[1], "r");
	assert(input != NULL);

	fileParser(input, argv[1]);
	// initialize the latches
	IFIDLatch.instruction.opcode = 0;
	IFIDLatch.instruction.rs = 0;
	IFIDLatch.instruction.rt = 0;
	IFIDLatch.instruction.rd = 0;
	IFIDLatch.instruction.Imm = 0;
	IFIDLatch.instruction.result = 0;
	IFIDLatch.read = 0;
	IFIDLatch.write = 0;

	IDEXLatch.instruction.opcode = 0;
	IDEXLatch.instruction.rs = 0;
	IDEXLatch.instruction.rd = 0;
	IDEXLatch.instruction.rt = 0;
	IDEXLatch.instruction.Imm = 0;
	IFIDLatch.instruction.result = 0;
	IDEXLatch.read= 0;
	IDEXLatch.write = 0;
	
	EXMEMLatch.instruction.opcode = 0;
	EXMEMLatch.instruction.rs = 0;
	EXMEMLatch.instruction.rd = 0;
	EXMEMLatch.instruction.rt = 0;
	EXMEMLatch.instruction.Imm = 0;
	IFIDLatch.instruction.result = 0;
	EXMEMLatch.read = 0;
	EXMEMLatch.write = 0;
	
	MEMWBLatch.instruction.opcode = 0;
	MEMWBLatch.instruction.rs = 0;
	MEMWBLatch.instruction.rt = 0;
	MEMWBLatch.instruction.rd = 0;
	MEMWBLatch.instruction.Imm = 0;
	IFIDLatch.instruction.result = 0;
	MEMWBLatch.read = 0;
	MEMWBLatch.write = 0;
	
	
	
	fclose(input);
	return 0;
}

//////////////////////////////////////
//////The pipelines/////////////////
////////////////////////////////
void IF()
{
	if(IFIDLatch.write){ 

	
	}

}


void ID()
{
	
	// have to make sure that for mul sub and add that it doesn't interfere with future counters
	
	if(IDEXLatch.write && IFIDLatch.read){
		if((IFIDLatch.instruction.rs == IDEXLatch.instruction.rd)
			|| (IFIDLatch.instruction.rs == EXMEMLatch.instruction.rd)
				|| (IFIDLatch.instruction.rs == MEMWBLatch.instruction.rd))
		{
			IDEXLatch.read = 1;
			IDEXLatch.write = 0;
			
			
			
		}else if((IFIDLatch.instruction.rt == IDEXLatch.instruction.rd) 
					|| (IFIDLatch.instruction.rt == EXMEMLatch.instruction.rd) 
						|| (IFIDLatch.instruction.rt == MEMWBLatch.instruction.rd))
		{
			IDEXLatch.read = 1;
			IDEXLatch.write = 0;
		}
		switch(IFIDLatch.instruction.opcode){
			case add:
				IDEXLatch.instruction = IFIDLatch.instruction;
				IDEXLatch.read = 1;
				IDEXLatch.write = 0;
				IFIDLatch.write = 1;
				IFIDLatch.read = 0;
                //ID_cycle_count++;
				break;
			case sub:
				IDEXLatch.instruction = IFIDLatch.instruction;
				IDEXLatch.read = 1;
				IDEXLatch.write = 0;
				IFIDLatch.write = 1;
				IFIDLatch.read = 0;
                //ID_cycle_count++;
				break;
			case mult:
				IDEXLatch.instruction = IFIDLatch.instruction;
				IDEXLatch.read = 1;
				IDEXLatch.write = 0;
				IFIDLatch.write = 1;
				IFIDLatch.read = 0;
                //ID_cycle_count++;
				break;
			case addi||lw||sw:
				IFIDLatch.instruction.rs = registers[IFIDLatch.instruction.rs];
				IFIDLatch.instruction.Imm = registers[IFIDLatch.instruction.Imm];
				IDEXLatch = IFIDLatch;
				IDEXLatch.read = 1;
				IDEXLatch.write = 0;
				IFIDLatch.write = 1;
				IFIDLatch.read = 0;
				//EX_cycle_count++;
				break;	
			case beq: //i type
	            IDEX = IfId;
				IDEX.readyToRead = 1;
				IDEX.readytoWrite = 0;
				IFID.readytoWrite = 1;
				IFID.readyToRead = 0;
                //ID_cycle_count++;
		
			/*case haltSimulation:
				IDEX = IfId;
				IDEX.readyToRead = 1;
				IDEX.readytoWrite = 0;
				IFID.readytoWrite = 0;
				IFID.readyToRead = 0;
				return 0;//return IfId;
				
			case noop:
				IDEX = IfId;
				IDEX.readyToRead = 1;
				IDEX.readytoWrite = 0;
				IFID.readytoWrite = 1;
				IFID.readyToRead = 0;
				return 0;//return IfId;
			*/
		}
	}
}

void EX()
{
	if(IDEXLatch.read && EXMEMLatch.write)
	{
		switch(IDEXLatch.instruction.opcode)
		{
			case add:
				IDEXLatch.instruction.result = registers[IDEXLatch.instruction.rs] + registers[IDEXLatch.instruction.rt];
				EXMEMLatch.read = 1;
				EXMEMLatch.write = 0;
				IDEXLatch.write = 1;
				IDEXLatch.read = 0;
				EXMEMLatch.instruction = IDEXLatch.instruction;
				break;
				
			case sub:
				IDEXLatch.instruction.result = registers[IDEXLatch.instruction.rs] - registers[IDEXLatch.instruction.rt];
				EXMEMLatch.read = 1;
				EXMEMLatch.write = 0;
				IDEXLatch.write = 1;
				IDEXLatch.read = 0;
				EXMEMLatch.instruction = IDEXLatch.instruction;
				break;

			case mult:
				IDEXLatch.instruction.result = registers[IDEXLatch.instruction.rs] * registers[IDEXLatch.instruction.rt];
				EXMEMLatch.read = 1;
				EXMEMLatch.write = 0;
				IDEXLatch.write = 1;
				IDEXLatch.read = 0;
				EXMEMLatch.instruction = IDEXLatch.instruction;
				break;
				
			case addi||lw||sw:
				IDEXLatch.instruction.result = registers[IDEXLatch.instruction.rs] + registers[IDEXLatch.instruction.Imm];
				EXMEMLatch.read = 1;
				EXMEMLatch.write = 0;
				IDEXLatch.write = 1;
				IDEXLatch.read = 0;
				EXMEMLatch.instruction = IDEXLatch.instruction;
				break;
			
			case beq:
				if(registers[IDEXLatch.instruction.rs]==registers[IDEXLatch.instruction.rt]){
					PC = PC+4+registers[IDEXLatch.instruction.Imm];
				}
				EXMEMLatch.read = 1;
				EXMEMLatch.write = 0;
				IDEXLatch.write = 1;
				IDEXLatch.read = 0;
				EXMEMLatch.instruction = IDEXLatch.instruction;
				break;
	
		
		}
	}
}


void MEM()
{
	//c cycles
	
	int address;
	
	//sign immediate and register to find address 
	address = EXMEMLatch.instruction.rs+EXMEMLatch.instruction.Imm;
	if(EXMEMLatch.read && MEMWBLatch.write)
		{
		if(EXMEMLatch.instruction.opcode == lw)
		{	
			registers[EXMEMLatch.instruction.rt] = dataMemory[address];
	
		}
		else if (EXMEMLatch.instruction.opcode == sw)
		{
			dataMemory[address] = EXMEMLatch.instruction.rt;
		}
		else
		{
			printf("error Bill Leonard");
		}
		MEMWBLatch.instruction = EXMEMLatch.instruction;
			MEMWBLatch.read = 1;
			MEMWBLatch.write = 0;
			EXMEMLatch.read = 0;
			EXMEMLatch.write = 1;
	}
}
void WB()
{
	//c cycles
	int opcode = MEMWBLatch.instruction.opcode; 
	if(MEMWBLatch.read){
	if(opcode == add || opcode == sub || opcode == addi || opcode == mult)
	{
		// get actual value to put in register
		registers[MEMWBLatch.instruction.rd] = MEMWBLatch.instruction.result;
		
	}else
	{
		printf("error Bill Leonard");
	}
		MEMWBLatch.write = 1;
		MEMWBLatch.read = 0;
	}
}


