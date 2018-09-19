first: all

ifneq ($(OS),Windows_NT)
OS := $(shell uname -s)
endif

C_PP_FLAGS += -I.
# C_PP_FLAGS += -DMEM_DEBUG=0
C_PP_FLAGS += -DMEM_DEBUG=1

ifeq ($(OS),Linux)
C_PP_FLAGS += -D_GNU_SOURCE
C_PP_FLAGS += -D_POSIX_SOURCE
endif

C_CC_FLAGS += --std=c99
C_CC_FLAGS += -g
C_CC_FLAGS += -Wall -Wextra
C_CC_FLAGS += -Werror

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
	$(CC) $(C_PP_FLAGS) -c $(C_CC_FLAGS) -o $@ $<

$(LIBNAME): $(C_LIB_OBJ)
	ar cr $@ $^

gonzo: gonzo.o $(LIBNAME)
	$(CC) $(C_CC_FLAGS) -o$@ $< -L. -l$(LIBRARY)

repl: repl.o $(LIBNAME)
	$(CC) $(C_CC_FLAGS) -o$@ $< -L. -l$(LIBRARY)

all: gonzo repl

clean:
	rm -f $(LIBNAME)
	rm -f gonzo repl
	rm -f *.o
	rm -f *~
