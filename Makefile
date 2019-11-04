CC = clang
CFLAGS = -g
#DEBUG = -DBINARYDEBUG

SRCS = linkedlist.c main.c talloc.c lib/tokenizer.o parser.c interpreter.c
HDRS = linkedlist.h value.h talloc.h tokenizer.h parser.h interpreter.h
OBJS = $(SRCS:.c=.o)

.PHONY: interpreter   # forces a rebuild always
interpreter: $(OBJS)
	$(CC) -rdynamic $(CFLAGS) $^  -o $@

.PHONY: phony_target
phony_target:

%.o : %.c $(HDRS) phony_target
	$(CC)  $(CFLAGS) $(DEBUG) -c $<  -o $@

.PHONY: clean
clean:
	rm *.o
	rm interpreter
