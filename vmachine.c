#include <string.h>
#include <lexer.h>
#include <parser.h>
#include <stdio.h>
#include <stdlib.h>
#include <vmachine.h>

#define MAXSTACKSIZE 1024

//int lookahead; // anticipated read of a token to match the grammar token

//global sematic variables and functions which emulate accumulator register
double acc = 0;
int mem_error = 0;

//stack structures and methods
double stack[MAXSTACKSIZE];
int sp = -1;
void push(void){
	sp++;
	stack[sp] = acc;
	return;
}

double pop(void){
	double aux = stack[sp];
	sp--;
	return aux;
}

// symbol table and virtual memory
#define SYMTAB_MAXENTRIES 1024
char symtab[SYMTAB_MAXENTRIES][MAXSTRLEN+1];
int symtab_nextentry = 0;

//virtual memory properly said, associates symbol with its memory address
double vmem[SYMTAB_MAXENTRIES];

void report_memory_error(void)
{
    if((symtab_nextentry + 1) > SYMTAB_MAXENTRIES){
		fprintf(stderr, "symtab overflow");
	}
}

int symtab_lookup(char *query_symbol){
	for(int i = 0; i < symtab_nextentry; i++){
		if(strcmp(symtab[i],query_symbol) == 0){
			return i;
		}
	}
	return -1;
}

int symtab_append(char *new_symbol){
	// treat new_symbol as string
	if((symtab_nextentry + 1) > SYMTAB_MAXENTRIES){
		mem_error = -1;
		report_memory_error();
		return -1; //surpases boundary
		// ERROR CODE -> add this once its done!
	}
	strcpy(symtab[symtab_nextentry], new_symbol);
	vmem[symtab_nextentry] = acc; //acc has adress, matches the L-value stroing process
	return symtab_nextentry++;
	// should check symtab boundary beforehand (error code)
}

//semantic functions to perform arithmetic operations and memory recall/store
void store(char *varname){ // varname := acc
	int entry_found = symtab_lookup(varname);
	if(entry_found == -1){
		// needs entry_found to store in vmem
		entry_found = symtab_append(varname);
	} 
	if(entry_found != -1){
		vmem[entry_found] = acc;
	}
} 

double recall(char *varname){
	int entry_found = symtab_lookup(varname);
	if(entry_found == -1){
		entry_found = symtab_append(varname);
		if(entry_found != -1){
			vmem[entry_found] = 0;
		}
	}
	if(entry_found != -1){
		return vmem[entry_found];
	}
	return 0;
}