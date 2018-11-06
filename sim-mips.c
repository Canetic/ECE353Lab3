/*
 ============================================================================
 Name        : sim-mips.c
 Author      : Angus Mo, Karl Shao, Timothy Shum, O-Dom Pin
 Version     :
 Description : MIPS data path simulator that can run in both single cycle and
 	 	 	   batch mode
 ============================================================================
 */

// Author: Shikang Xu; ECE 353 TA

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

//feel free to add here any additional library names you may need
#define SINGLE 1
#define BATCH 0
#define REG_NUM 32
main (int argc, char *argv[]){
	int sim_mode = 0; //mode flag, 1 for single-cycle, 0 for batch
	int c, m, n;
	int i; //for loop counter
	long mips_reg[REG_NUM];
	long pgm_c = 0; //program counter
	long sim_cycle = 0; //simulation cycle counter
	//define your own counter for the usage of each pipeline stage here

	int test_counter = 0;
	FILE *input = NULL;
	FILE *output = NULL;
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
	//initialize registers and program counter
	if(sim_mode==1){
		for (i=0;i<REG_NUM;i++){
			mips_reg[i]=0;
		}
	}

	//start your code from here
}
