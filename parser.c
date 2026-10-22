#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <tokens.h>
#include <lexer.h>
#include <parser.h>
#include <stdbool.h>
#include <vmachine.h>
#include <main.h>


int lookahead; // anticipated read of a token to match the grammar token
int parser_error = 0; // parser_error, lower than 0 indicates error



// syntax function of mybc interpretor 
// semantic actions are within [ ]
/*
[0] -> E
E -> [1] (isneg [2])? T [3] (oplus [4] T [5])*
T -> F (otimes [6] F [7])*
F -> FLT [8]
   | ID [9] (':=' E [10] | [11])
   | '(' E ')'
*/
void Expre(void)
{
	if (parser_error)
		return;

	// semantic variables:
	int isnegate = false;
	int oplus = 0;
	int otimes = 0;
	//varname stores in lexeme
	char varname[MAXSTRLEN+1];

	if (lookahead == '+' || lookahead == '-') {
		// semantic action [2] -> save value of isneg, if it exists, in isnegate.t
		if(lookahead == '-'){
			isnegate = true;
		}
		match(lookahead);
		if (parser_error)
			return;
	}

_T: // term begin



_F: // factor begin
	switch(lookahead) {
		//case DEC: match(DEC); 
		//	acc = atoi(lexeme);
		//	break;
		case FLT:
			acc = atof(lexeme);
			match(FLT);
			if (parser_error)
				return;
			break;
		//case OCT: match(OCT); 
		//	break;
		//case HEX: match(HEX); 
		//	break;
		case ID:
			// semantic expression [9] -> save variable name from the lexeme as to avoid overwrite
			strcpy(varname, lexeme); //save variable name, from a lexeme, to varname, lexeme is cleaned after match 
			match(ID);
			if (parser_error)
				return;
			if (lookahead == ASGN) {
				// semantic expression [10] -> save acc in memory through ID
				// at this juncture, we have F -> ID := E to a virtual memory position (store)
				// Usually, this operation is called L-Value
				match(ASGN);
				if (parser_error)
					return;
				Expre();
				if (parser_error)
					return;
				store(varname); 
			} else {
				// semantic expression [11] -> recall value of selected ID, put in acc
				acc = recall(varname); 	
					// just recalling ID from the virtual memory (recall)
					// acc <- val(mem_position);
					// this one is called R-Value
			}
			break;
		default:
			match('(');
			if (parser_error)
				return;
			Expre();
			if (parser_error)
				return;
			match(')');
			if (parser_error)
				return;
	}

	if (parser_error)
		return;

	// are there pending / or * opperations?
	if (otimes){ // semantic action [7] -> print value of otimes.t and recover F from stack
		// Reduction
		if (otimes == '*'){
			acc = pop() * acc;
		}
		if (otimes == '/'){
			acc = pop() / acc;	
		}
		otimes = 0;
	}

	// if (lookahead == otimes)
	if (lookahead == '*' || lookahead == '/') { 
		otimes = lookahead; // semantic action [6] -> store value of otimes in otimes.t
		push();
		match(lookahead);
		if (parser_error)
			return;
		goto _F;
	}

	// product end
	if (isnegate) { 
		acc = -acc; // semantic action [3] -> apply isnegate to acc if detected
		isnegate = 0; // semantic action [1] -> set value of is negate to 0
	}
	
	if (oplus){
		// Reduction
		// semantic action [5] -> print value of oplus.t and recover T from stack
		if (oplus == '+'){
			acc = pop() + acc;
		}
		if (oplus == '-'){
			acc = pop() - acc;
		}
		oplus = 0;
	}

	// if (lookahead == oplus)
	if (lookahead == '+' || lookahead == '-') {
		oplus = lookahead; // semantic action [4] -> save value of oplus in oplus.t and T in stack
		push();
		match(lookahead);
		if (parser_error)
			return;
		goto _T;
	}

	// sum end
}

// report syntax error without exiting the program, just set parser_error flag to <0
static void report_syntax_error(void)
{
	if (lookahead >= 0 && lookahead <= UCHAR_MAX &&
		isprint((unsigned char)lookahead)) {
        fprintf(stderr, "token %c mismatch at line %d\n",
				lookahead, linenumber);
    } else {
        fprintf(stderr, "token %d mismatch at line %d\n",
				lookahead, linenumber);
    }
}

// match is the first interface connecting lexer and parser
void match(int required)
{
	if (parser_error)
		return;

	if (lookahead == required) {
		lookahead = gettoken(source);
	} else {
		report_syntax_error();
		parser_error = -1;
	}
}

int isFIRST(void){
	switch(lookahead){
		case '+': case '-': case ID: case FLT: case '(': return lookahead;
	}
	return 0;
}


void cli(void)
{
	while (lookahead != EOF) {
		while (lookahead == '\n' || lookahead == ';'){
			lookahead = gettoken(source); //gettoken aready does linenumber++
		}

		if (lookahead == EOF)
			break;

		parser_error = 0; // sets parser and memory error
		//mem_error = 0;
		sp = -1;

		if (isFIRST()) {
			Expre();
		} else {
			report_syntax_error();
			parser_error = -1;
		}

		if (!parser_error && lookahead != EOF &&
		    lookahead != '\n' && lookahead != ';') {
			report_syntax_error();
			parser_error = -1;
		}
		

		if (parser_error < 0 || mem_error < 0) {
			while (lookahead != EOF &&
			       lookahead != '\n' && lookahead != ';') {
				lookahead = gettoken(source);
			}

			if (lookahead != EOF)
				lookahead = gettoken(source);

			sp = -1;
			continue;
		}

		fprintf(objfile, "%lg\n", acc);
		if (lookahead == '\n' || lookahead == ';')
			lookahead = gettoken(source);
	}
}

