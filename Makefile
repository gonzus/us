first: libus.a

CFLAGS += -g
CFLAGS += -I.

cell.o: cell.c
env.o: env.c

libus.a: cell.o env.o
	ar cr $@ $^

gonzo.o: gonzo.c

gonzo: gonzo.o libus.a
	$(CC) $(CFLAGS) -o$@ gonzo.o -L. -lus

clean:
	rm -f *.o *.a *~
	rm -f gonzo
