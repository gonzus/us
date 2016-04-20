first: all

CFLAGS += -g
CFLAGS += -I.
CFLAGS += -Wall
CFLAGS += -Werror

cell.o: cell.c
env.o: env.c
parser.o: parser.c
eval.o: eval.c

libus.a: cell.o env.o parser.o eval.o
	ar cr $@ $^

gonzo.o: gonzo.c

gonzo: gonzo.o libus.a
	$(CC) $(CFLAGS) -o$@ gonzo.o -L. -lus

all: libus.a

clean:
	rm -f *.o *.a *~
	rm -f gonzo
