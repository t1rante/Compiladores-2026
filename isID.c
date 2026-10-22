#include <stdio.h>
#include <ctype.h>

#define ID	1024
// ID = [A-Za-z][A-Za-z0-9]
int isID(FILE *tape)
{
	int head = getc(tape);

	if (isalpha(head)) {
		while(isalnum(head = getc(tape)));
		ungetc(head, tape);
		return ID;
	}
	ungetc(head, tape);

	return 0;
}

int main(void)
{
	FILE *buffer = stdin;
	int token;
	if ( (token = isID(buffer)) ) {
		printf("ID\n");
		return 0;
	}
	printf("Not an identifier\n");
	return -1;
}
