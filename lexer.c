#include <stdio.h>
#include <ctype.h>
#include <tokens.h>
#include <lexer.h>
#include <string.h>
#include <vmachine.h>

int linenumber = 1;

/*
	2026-09-08: Implementem um varredor de comentários Pascal [void skipcomments(FILE *)], onde
	um comentário começa com '{' e finaliza com '}', a menos que se utilize
	"\{" e "\}", para ser interpretado como elemento do texto.
	Example:
	begin { isto é um comentário. Aqui, eu devo utilizar \{ e \} para constar no interior
		    do comentário
		  }
	  blah blah blah
	end;
 */

	#include <stdlib.h>
char lexeme[MAXSTRLEN+1];

void skipspaces(FILE *tape)
{
	int head;

	while ( isspace(head = getc(tape)) ) {
		if (head == '\n') {
		linenumber++;
		break;
		}
	}
	
	ungetc(head, tape);
} 

// check if it works: skipcomment
void skipscomment(FILE *tape)
{
	int head = getc(tape); 
_skipspc: 
	skipspaces(tape);
	
	if( head == '{'){ // must start comment with
		while((head = getc(tape)) != '}'){ 
			if( head == '\n'){
				linenumber++;
			}; // linebreaks from \n in comment counts as linenumber!
	
			if(head == '{'){
				fprintf(stderr,"Error -4: { inside a comment");
				exit(-4); //error?! -4 -> 
			}

			if(head == '\\'){ // ignore next character inside comment, } doesn't count
				head = getc(tape);
			}
		}
		//printf("head = %c", head);
	} else {
		//printf("head = %c\n", head);
		if(head != '}'){
			ungetc(head, tape);
		}
		return;
	}
	goto _skipspc;
}


// ASGN = ":="
int isASGN(FILE *tape)
{
	if ((lexeme[0] = getc(tape)) == ':') {
		if ( (lexeme[1] = getc(tape)) == '=' ) {
			lexeme[2] = 0;
			return ASGN;
		}
		ungetc(lexeme[1], tape);
	}
	ungetc(lexeme[0], tape);
	lexeme[0] = 0;
	return 0;
}

// ID = [A-Za-z][A-Za-z0-9]*

int isID(FILE *tape)
{

	if (isalpha((lexeme[0] = getc(tape)))) {
		int i = 1;
		while(isalnum(lexeme[i] = getc(tape))){
			if (i < MAXSTRLEN) {
				i++;
			}
		}
		ungetc(lexeme[i], tape);
		lexeme[i] = 0;
		return ID;
	}
	ungetc(lexeme[0], tape);
	return 0;
}

// DEC = 0 | [1-9][0-9]*

int isDEC(FILE *tape)
{
	lexeme[0] = getc(tape);

	if (isdigit(lexeme[0])) {
		if (lexeme[0] == '0') {
			lexeme[1] = 0;
			return DEC;
		}
		int i = 1;
		while( isdigit(lexeme[i] = getc(tape)) )
		{
			i++;
		}
		ungetc(lexeme[i], tape);
		lexeme[i] = 0;
		return DEC;
	}

	ungetc(lexeme[0], tape);
	lexeme[0] = 0; //emulate epsilon
	return 0;
}

// OCT = 0[0-7]+
int isOCT(FILE *tape)
{
	int head, prfx;

	if( (prfx = getc(tape)) == '0') {

		if ( '0' <= (head = getc(tape)) && head <= '7' ) {

			while ( '0' <= (head = getc(tape)) && head <= '7' );
			ungetc(head, tape);
			return OCT;

		}

		ungetc(head, tape);
	}

	ungetc(prfx, tape);
	return 0;
}

// HEX = 0[xX][0-9A-Fa-f]+
int isHEX(FILE *tape)
{
	char hexpref[2];
	if ( (hexpref[0] = getc(tape)) != '0' ) {
		ungetc(hexpref[0], tape);
		return 0;
	}
	// by now we got '0'
	if ( (toupper(hexpref[1]) != 'X') ) {
		ungetc(hexpref[1], tape);
		ungetc(hexpref[0], tape);
		return 0;
	}

	// at this point we got prefix "0[xX]"
	int head = getc(tape);

	// check wether head is in [0-9A-Fa-f]
	if (isxdigit(head)) {
		while ( isxdigit(head = getc(tape)) );
		ungetc(head, tape);
		return HEX;
	}

	// give back what is not a hex-digit
	ungetc(head, tape);

	// give back the hex prefix
	ungetc(hexpref[1], tape);
	ungetc(hexpref[0], tape);
	return 0;
}

// EE = [eE][+-]?[0-9]+
int isEE(FILE *tape, int *lexPos)
{
    int head;
    int start = *lexPos;
	
    head = getc(tape);

    if (head != 'e' && head != 'E') {
        if (head != EOF)
            ungetc(head, tape);

        return 0;
    }

    lexeme[++(*lexPos)] = (char)head;

    head = getc(tape);

    if (head == '+' || head == '-') {
        lexeme[++(*lexPos)] = (char)head;
        head = getc(tape);
    }

    int has_digits = 0;

    while (head >= '0' && head <= '9') {
        lexeme[++(*lexPos)] = (char)head;
        has_digits = 1;
        head = getc(tape);
    }

    // A char that end a Expo is NOT part of expo!
    if (head != EOF)
        ungetc(head, tape);

    if (!has_digits) {
        // Deals with invalid expo
        for (int i = *lexPos; i > start; i--)
            ungetc(lexeme[i], tape);

        *lexPos = start;
        return 0;
    }
    return 1;
}

// FLT = DEC EE | [0 - 9]* "." [0 - 9]+ EE? | [0 - 9]+ \. [0 - 9]* EE?
// ie: 3e8 3. .123 787.1293 123.574e-12

int isFLT(FILE *tape){
    double value;
    int head = getc(tape);
    if(isdigit(head) || head == '.'){ //make sure that it starts with a number or .
        ungetc(head, tape);
        int nc = fscanf(tape, "%lg", &value); 
        if(nc){
            sprintf(lexeme, "%lg", value);
            return FLT;
        }
    }
    ungetc(head, tape);
    return 0;
}

/*
int isFLT(FILE *tape)
{
	//check if it is a float number
    int first = getc(tape);
    if (first != '.' &&
        (first == EOF || !isdigit((unsigned char)first))) {
        if (first != EOF)
            ungetc(first, tape);
        return 0;
    }

	// put back the first character to read the float number with fscanf
    ungetc(first, tape);

    double value;
	// use %lg to read a floating-point number in either decimal or scientific notation
    if (fscanf(tape, "%lg", &value) == 1) {
        sprintf(lexeme, "%lg", value);
        return FLT;
    }
    return 0;
}
*/

int gettoken(FILE *src)
{
	int token;

	skipscomment(src);
	if ((token = isASGN(src))) return token;
	if ((token = isID(src))) return token;
	if ((token = isFLT(src))) return token;

	return getc(src);
}
