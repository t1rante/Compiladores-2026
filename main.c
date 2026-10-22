#include <stdio.h>
#include <ctype.h>
#include <main.h>
#include <lexer.h>
#include <vmachine.h>

FILE *source;
FILE *objfile;  

int main(void)
{
		source = stdin;
		objfile = stdout;
		lookahead = gettoken(source);
		
		cli();
		
		//fprintf(objfile, "last acc: %lg", acc);

		if(lookahead == EOF) {
			fprintf(stdout,"\n");
			return 0;	
		}
		
}
