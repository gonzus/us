first: all

CFLAGS += -g
CFLAGS += -I.
CFLAGS += -Wall
CFLAGS += -Werror

C_LIB_SRC = \
	log.c \
	cell.c \
	env.c \
	parser.c \
	eval.c \
	native.c \

C_EXE_SRC = \
	gonzo.c \

LIBRARY = us

C_LIB_HDR = $(C_LIB_SRC:.c=.h)
C_LIB_OBJ = $(C_LIB_SRC:.c=.o)
C_EXE_OBJ = $(C_EXE_SRC:.c=.o)
LIBNAME = lib$(LIBRARY).a
EXENAME = $(C_EXE_SRC:.c=)

%.o: %.c $(C_LIB_HDR)
	$(CC) -c $(CFLAGS) -o $@ $<

$(LIBNAME): $(C_LIB_OBJ)
	ar cr $@ $^

$(EXENAME): $(C_EXE_OBJ) $(LIBNAME)
	$(CC) $(CFLAGS) -o$@ $< -L. -l$(LIBRARY)

all: $(EXENAME)

clean:
	rm -f $(EXENAME)
	rm -f $(LIBNAME)
	rm -f $(C_LIB_OBJ) $(C_EXE_OBJ)
	rm -f *~
