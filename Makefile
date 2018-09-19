first: all

# CFLAGS += -DMEM_DEBUG=0
CFLAGS += -DMEM_DEBUG=1
CFLAGS += -g
CFLAGS += -I.
CFLAGS += -Wall
CFLAGS += -Wextra

C_LIB_SRC = \
	log.c \
	mem.c \
	arena.c \
	cell.c \
	env.c \
	parser.c \
	eval.c \
	native.c \
	us.c \

LIBRARY = us

C_LIB_HDR = $(C_LIB_SRC:.c=.h)
C_LIB_OBJ = $(C_LIB_SRC:.c=.o)
LIBNAME = lib$(LIBRARY).a

%.o: %.c $(C_LIB_HDR)
	$(CC) -c $(CFLAGS) -o $@ $<

$(LIBNAME): $(C_LIB_OBJ)
	ar cr $@ $^

gonzo: gonzo.o $(LIBNAME)
	$(CC) $(CFLAGS) -o$@ $< -L. -l$(LIBRARY)

#$(CC) $(CFLAGS) -o$@ $< -L. -l$(LIBRARY) -L ../electric-fence -lefence

repl: repl.o $(LIBNAME)
	$(CC) $(CFLAGS) -o$@ $< -L. -l$(LIBRARY)

all: gonzo repl

clean:
	rm -f $(LIBNAME)
	rm -f gonzo repl
	rm -f *.o
	rm -f *~
