/*
 ============================================================================
 Name        : sim-mips.c
 Author      : Angus Mo, Karl Shao, Timothy Shum, O-Dom Pin
 Version     :
 Description : MIPS data path simulator that can run in both single cycle and
 	 	 	   batch mode
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <limits.h>

#define SINGLE 1
#define BATCH 0
#define REG_NUM 32
#define MAX_ADDR 512

enum op {add=0x20, addi=0x8, sub=0x22, mult=0x18, beq=0x4, lw=0x23,
	sw=0x2b, haltsimulation=0xff};
int errorCode = 0;
int maxIMAddress;
int IMAddress;
int sim_mode=0;//mode flag, 1 for single-cycle, 0 for batch
int c,m,n;
int i;//for loop counter
int exCounter;
int memCounter;
long mips_reg[REG_NUM];
long pgm_c=0;//program counter
long sim_cycle=0;//simulation cycle counter
long ifUsed, idUsed, exUsed, memUsed, wbUsed;
float  ifUtil, idUtil, exUtil, memUtil, wbUtil;
int stall;

struct inst
{
	int opcode;
	unsigned int rs;
	unsigned int rt;
	unsigned int rd;
	int Imm;
};

struct stateReg
{
	//IfId
	struct inst instruction;
	//IdEx
	int rsData;
	int rtData;
	int rtAddr;
	int rdAddr;
	int ImmData;
	int exCounter;
	//ExMem
	int aluResult;
	int memCounter;
	int destination;
	//MemWb
	int memData;
	int memWriteAddr;
	int writeData;
};



struct inst instMem[MAX_ADDR];
int dataMem[MAX_ADDR];
struct stateReg IfId, IdEx, ExMem, MemWb, empty;

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
		if((Imm >= SHRT_MIN) && (Imm < SHRT_MAX)){
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
	//match variables
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
		p = p1&p2;
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
	retLine = (char *)malloc(100*sizeof(char));
	tokens = (char **)malloc(10*sizeof(char *));
	//decalre the delimeters
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
	if(((atoi(token) <= 31) && (atoi(token) >= 10)))
	{
		regNum = atoi(token);
	} else {
		//determine the register value if it is symbolic
		switch(token[0])
		{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if (strlen(token) == 1)
			{
				regNum = atoi(token);
			}
			break;
		case 'a':
			switch(token[1])
			{
			//Assembler temporary
			case 't':
				regNum = 1;
				break;
				//Arguments
			case '0':
				regNum = 4;
				break;
			case '1':
				regNum = 5;
				break;
			case '2':
				regNum = 6;
				break;
			case '3':
				regNum = 7;
				break;
			default:
				break;
			}
			break;
			//Frame pointer
			case 'f':
				if (token[1] == 'p')
					regNum = 30;
				break;
				//Global pointer
			case 'g':
				if (token[1] == 'p')
					regNum = 28;
				break;
				//reserved for OS kernel
			case 'k':
				switch(token[1])
				{
				case '0':
					regNum = 26;
					break;
				case '1':
					regNum = 27;
					break;
				default:
					break;
				}
				break;
				//Return address
				case 'r':
					if (token[1] == 'a')
						regNum = 31;
					break;
				case 's':
					switch(token[1])
					{
					//Stack pointer
					case 'p':
						regNum = 29;
						break;
						//Saved temporaries
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
						//Function or expression result
						case 'v':
							switch(token[1])
							{
							case '0':
								regNum = 2;
								break;
							case '1':
								regNum = 3;
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
		if((strcmp(opField+1, "altSimulation") == 0) && (strtok(NULL, " ") == NULL)){
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
	int lineNum;

	lineNum = 1;
	maxIMAddress = 0;
	while(fgets(line, 100, fp)){
		fmtLine = (char *)malloc(100*sizeof(char));
		//format the input instruction
		fmtLine = progScanner(strdup(line));
		//continue only if the formatted string isn't NULL
		assert(fmtLine != NULL);

		//load valid instructions into the instruction memory if it isn't full
		if((strcmp(fmtLine, "")!=0) && (maxIMAddress < 512)){
			instruction = parser(fmtLine);
			instMem[maxIMAddress++] = instruction;
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
				puts("Immediate must be and integer between -32,768 and +32,767");
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

// fetches instruction and puts it into IFID Latch
void IF()
{
	//increment prog_c if the cpu did not stall in later stages

	if (!stall)
	{

		if(pgm_c < maxIMAddress){
			IfId.instruction = instMem[pgm_c++];
			if(IfId.instruction.opcode != haltsimulation){
				ifUsed++;
			}
		}
		assert(pgm_c <= maxIMAddress); // makes sure program counter does not exceed instructions in memory

	}
	stall = 0;

}

void ID()
{
	//check for RAW hazards
	switch(IfId.instruction.opcode)
	{
	case add:
	case mult:
	case sub:
	case beq:
	case sw:
		//make sure that the instruction's rs doesn't use the rd
		if ((IfId.instruction.rs==MemWb.destination)||(IfId.instruction.rs==ExMem.destination))
			stall = 1;
		//make sure that the instruction's rt doesn't use the rd
		if ((IfId.instruction.rt==MemWb.destination)||(IfId.instruction.rt==ExMem.destination))
			stall = 1;
		break;
	case addi:
	case lw:
		//		if (IfId.instruction.rs==MemWb.destination)
		//					stall = 1;
		break;
	default:
		break;
	}
	//if its a zero register no need to stall
	if ((MemWb.destination==0)||(0==ExMem.destination))
		stall=0;

	if (!stall)
	{
		//idUsed++;
		//update excounter here
		switch(IfId.instruction.opcode)
		{
		case add:
		case sub:
			exCounter = n;
			IdEx.rsData = mips_reg[IfId.instruction.rs];		//get the register's data at that register
			IdEx.rtData = mips_reg[IfId.instruction.rt];
			IdEx.rdAddr = IfId.instruction.rd;			// put the rd register into the next latch
			idUsed++;						// id is used so add +1
			break;
		case mult:
			exCounter = m;
			IdEx.rsData = mips_reg[IfId.instruction.rs];		//get the register's data at that register
			IdEx.rtData = mips_reg[IfId.instruction.rt];
			IdEx.rdAddr = IfId.instruction.rd;			// put the rd register into the next latch
			idUsed++;						// id is used so add +1
			break;
		case addi:
		case lw:
			exCounter = n;
			IdEx.rsData = mips_reg[IfId.instruction.rs];		//get the register's data at that register
			IdEx.ImmData = IfId.instruction.Imm;			// transfer the IfId's Imm to IdEx's ImmData latch
			IdEx.rtAddr = IfId.instruction.rt;			// put the rd register into the next latch
			idUsed++;						// id is used so add +1
			break;
		case sw:
		case beq:
			exCounter = n;
			IdEx.rsData = mips_reg[IfId.instruction.rs];		//get the register's data at that register
			IdEx.rtData = mips_reg[IfId.instruction.rt];		// transfer the IfId's Imm to IdEx's ImmData latch
			IdEx.ImmData = IfId.instruction.Imm;
			idUsed++;						// id is used so add +1
			break;
		default:
			break;
		}
		IdEx.instruction = IfId.instruction;			//transfer the instruction to the next latch
		IfId = empty;						// reset the IfId latch
	}
	// have to stall if the opcode is beq because of the branching
	if ((IdEx.instruction.opcode == beq)||(ExMem.instruction.opcode == beq))
		stall = 1;

}

void EX()
{
	//simulate execution time
	exCounter--;
	if (exCounter > 0){					`
		exUsed++;
		stall = 1;
	}

	//execute function
	if (!stall)
	{
		
		//run the execution for the different opcodes
		switch(IdEx.instruction.opcode)
		{
		case add:
			ExMem.aluResult = IdEx.rsData + IdEx.rtData; 	// adding rs and rt data
			ExMem.rdAddr = IdEx.rdAddr;			// transfer the rdAddr from IdEx to ExMem
			ExMem.destination = ExMem.rdAddr;		
			exUsed++;					// ex is used so add +1
			break;
		case mult:
			ExMem.aluResult = IdEx.rsData * IdEx.rtData;	// multiply rs and rt data		
			ExMem.rdAddr = IdEx.rdAddr;			// transfer the rdAddr from IdEx to ExMem
			ExMem.destination = ExMem.rdAddr;
			exUsed++;					// ex is used so add +1
			break;
		case sub:
			ExMem.aluResult = IdEx.rsData - IdEx.rtData;	// subtract rs and rt data
			ExMem.rdAddr = IdEx.rdAddr;			// transfer the rdAddr from IdEx to ExMem
			ExMem.destination = ExMem.rdAddr;
			exUsed++;					// ex is used so add +1
			break;
		case beq:	//subtract and compare to zero to check for equality
			ExMem.aluResult = IdEx.rsData - IdEx.rtData;
			ExMem.ImmData = IdEx.ImmData;
			if(ExMem.aluResult == 0)
			{
				pgm_c+= ExMem.ImmData;
				assert((pgm_c < maxIMAddress) && (pgm_c>=0));		// make sure that the program counter doesn't gp negative nor overflow
				IfId = empty;
				IdEx = empty;
				ExMem = empty;
				MemWb = empty;
			}
			exUsed++;
			break;
		case addi:	//determine added results
			ExMem.rtAddr = IdEx.rtAddr;
			ExMem.aluResult = IdEx.rsData + IdEx.ImmData;
			ExMem.destination = ExMem.rtAddr;
			exUsed++;
			break;
		case lw:	//determine address result
			ExMem.rtAddr = IdEx.rtAddr;
			ExMem.aluResult = IdEx.rsData + IdEx.ImmData;
			assert(ExMem.aluResult%4==0);
			ExMem.destination = ExMem.rtAddr;
			memCounter = c;
			exUsed++;
			break;
		case sw:	//determine address result
			ExMem.rtData = IdEx.rtData;
			ExMem.aluResult = IdEx.rsData + IdEx.ImmData;
			assert(ExMem.aluResult%4==0);
			memCounter = c;
			exUsed++;
			break;
		default:

			break;
		}
		ExMem.instruction = IdEx.instruction;			//transfer IdEx instructions to ExMem
		IdEx = empty;						// reset the IdEx
	}


}

// Accesses and stores addresses
void Mem()
{


	//simulate memory time
	memCounter--;
	if (memCounter > 0){
		memUsed++;}
	if (memCounter > 0){
		stall = 1;}

	if (!stall)
	{

		// forwards instructions as well as stores and loads from memory
		switch(ExMem.instruction.opcode)
		{
		case add:
		case mult:
		case sub:
			MemWb.rdAddr = ExMem.rdAddr;
			MemWb.aluResult = ExMem.aluResult;
			MemWb.destination = MemWb.rdAddr;

			break;
		case addi:
			MemWb.rtAddr = ExMem.rtAddr;
			MemWb.aluResult = ExMem.aluResult;
			MemWb.destination = MemWb.rtAddr;

			break;
		case lw: // loads value from memory
			assert((ExMem.aluResult<MAX_ADDR) && (ExMem.aluResult>=0)); // assertion to make sure memory address is within instruction memory
			MemWb.memData = dataMem[ExMem.aluResult];
			MemWb.rtAddr = ExMem.rtAddr;
			MemWb.destination = MemWb.rtAddr;
			memUsed++;
			break;
		case sw: // stores value into memory
			assert((ExMem.aluResult<MAX_ADDR) && (ExMem.aluResult>=0)); // assertion to make sure memory address is within instruction memory
			dataMem[ExMem.aluResult] = ExMem.rtData;
			memUsed++;
			break;
		case beq:
			break;
		default:

			break;
		}
		MemWb.instruction = ExMem.instruction; // forwards instruction
		ExMem = empty;
	}

}

// takes ALU computation and writes back into registers
void WB()
{


	// stores ALU value based on opcode
	switch(MemWb.instruction.opcode)
	{
	case add:
	case mult:
	case sub:
		wbUsed++;
		mips_reg[MemWb.rdAddr] = MemWb.aluResult;
		break;
	case addi:
		wbUsed++;
		mips_reg[MemWb.rtAddr] = MemWb.aluResult;
		break;
	case lw:
		wbUsed++;
		mips_reg[MemWb.rtAddr] = MemWb.memData;
		break;
	case sw:
	case beq:
		break;
	default:

		break;
	}
	if(MemWb.destination==0) // makes sure mips_reg[0] is always equal to 0
		mips_reg[0]=0;

	MemWb = empty;

}

int main(int argc, char *argv[])
{

	FILE *input=NULL;
	FILE *output=NULL;
	printf("The arguments are:");

	for(i=1;i<argc;i++){
		printf("%s ",argv[i]);
	}
	printf("\n");
	if(argc==7){
		if(strcmp("-s",argv[1])==0){
			sim_mode=SINGLE;
		}
		else if(strcmp("-b",argv[1])==0){
			sim_mode=BATCH;
		}
		else{
			printf("Wrong sim mode chosen\n");
			exit(0);
		}

		m=atoi(argv[2]);
		n=atoi(argv[3]);
		c=atoi(argv[4]);

		input=fopen(argv[5],"r");
		output=fopen(argv[6],"w");

	}

	else{
		printf("Usage: ./sim-mips -s m n c input_name output_name (single-sysle mode)\n or \n ./sim-mips -b m n c input_name  output_name(batch mode)\n");
		printf("m,n,c stand for number of cycles needed by multiplication, other operation, and memory access, respectively\n");
		exit(0);
	}
	if(input==NULL){
		printf("Unable to open input or output file\n");
		exit(0);
	}
	if(output==NULL){
		printf("Cannot create output file\n");
		exit(0);
	}

	if((m <= 0) || (n <= 0) || (c <= 0)){
		puts("number cycles cannot be negative or zero.");
		exit(0);
	}

	/////Instruction memory/////

	fileParser(input, argv[5]);
	IMAddress = 0;
	//initialize registers and program counter

	/////CPU/////
	for (i=0;i<REG_NUM;i++){
		mips_reg[i]=0;
	}

	//initialize empty state registers
	empty.instruction.opcode = 0;
	empty.rdAddr = -1;
	empty.rtAddr = -1;
	empty.destination = -1;

	IfId = empty;
	IdEx = empty;
	ExMem = empty;
	MemWb = empty;

	ifUsed = 0;
	idUsed = 0;
	exUsed = 0;
	memUsed = 0;
	wbUsed = 0;

	while(MemWb.instruction.opcode != haltsimulation)
	{
		WB();
		Mem();
		EX();
		ID();
		IF();

		if(sim_mode==SINGLE)
		{
			printf("cycle: %d register value: ",sim_cycle);
			for (i=1;i<REG_NUM;i++){
				printf("%d  ",mips_reg[i]);
			}
			printf("program counter: %d\n",pgm_c);
			printf("press ENTER to continue\n");
			while(getchar() != '\n');
		}
		sim_cycle++;
	}

	ifUtil = ifUsed/ (float) sim_cycle;
	idUtil = idUsed/ (float) sim_cycle;
	exUtil = exUsed/ (float) sim_cycle;
	memUtil = memUsed/ (float) sim_cycle;
	wbUtil = wbUsed/ (float) sim_cycle;

	if(sim_mode==BATCH){
		fprintf(output,"program name: %s\n",argv[5]);
		fprintf(output,"stage utilization: %f  %f  %f  %f  %f \n",
				ifUtil, idUtil, exUtil, memUtil, wbUtil);
		// add the (double) stage_counter/sim_cycle for each
		// stage following sequence IF ID EX MEM WB

		fprintf(output,"register values ");
		for (i=1;i<REG_NUM;i++){
			fprintf(output,"%d  ",mips_reg[i]);
		}
		fprintf(output,"program counter: %d\n",pgm_c);

	}
	//close input and output files at the end of the simulation
	fclose(input);
	fclose(output);
	return 0;
}
