#/usr/bin/make -f

# My Basic LL(1)-Calculator - mybc
# Creation date: 2026/08/17

CFLAGS = -g -I./

EXEC = mybc

$(EXEC): main.o lexer.o parser.o vmachine.o
	$(CC) -o $@ $^

clean:
	$(RM) *.o

mostlyclean: clean
	$(RM) *~ $(EXEC)
