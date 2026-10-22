
#define MAXSTACKSIZE 1024
#define SYMTAB_MAXENTRIES 1024
#define MAXSTRLEN 1024

extern double acc;
extern double stack[MAXSTACKSIZE];
extern int sp;
extern int mem_error;
extern void push(void);
extern double pop(void);
extern char symtab[SYMTAB_MAXENTRIES][MAXSTRLEN+1];
extern int symtab_nextentry;
extern double vmem[SYMTAB_MAXENTRIES];
extern int symtab_lookup(char *query_symbol);
extern int symtab_append(char *new_symbol);
extern void store(char *varname);
extern double recall(char *varname);
extern void report_memory_error(void);