# NIKKI RUDNICK
# nrudnick@pdx.edu

CC = gcc
DEBUG = -g3 -O0
DEFINES = 

WERROR = 
WERROR += -Werror
CFLAGS = $(DEBUG) -Wall -Wextra -Wshadow -Wunreachable-code -Wredundant-decls \
        -Wmissing-declarations -Wold-style-definition -Wmissing-prototypes \
        -Wdeclaration-after-statement -Wno-return-local-addr \
		-Wunsafe-loop-optimizations -Wuninitialized #$(WERROR) $(DEFINES)
LOCAL_INCLUDES = -I ~rchaney/argon2/include
LOCAL_LIBS = -L ~rchaney/argon2/lib/x86_64-linux-gnu -largon2

CFLAGS += $(LOCAL_INCLUDES)
LDFLAGS =
LDFLAGS += $(DEBUG)
PERMS = a+rx,g-w
PROG1 = vargon-threads
PROG2 = 
PROG3 = 
PROG4 = 
PROGS = $(PROG1) $(PROG2) $(PROG3) $(PROG4)

all: $(PROGS)

$(PROG1): $(PROG1).o
	$(CC) -o $@ $^ $(LDFLAGS) $(LOCAL_LIBS)
	chmod $(PERMS) $@

$(PROG1).o: $(PROG1).c
	$(CC) $(CFLAGS) -c $<

tar:
	tar cvfa $(PROG1).tar.gz *.[ch] ?akefile
	tar tvfa $(PROG1).tar.gz

# clean up the compiled files and editor chaff
clean cls:
	rm -f $(PROGS) *.o *~ \#* 

# You might prefer git
gotta git get gat:
	if [ ! -d .git ] ; then git init; fi
	git add *.[ch] ?akefile
	git commit -m"Gotta git that"

co:
	co -f -l RCS/*.[ch] RCS/?akefile*

test:
	shuf 10-argon-hashed.txt | ./$(PROG1) 

val valgrind:
	valgrind --leak-check=full --show-leak-kinds=all ./$(PROG1) < 10-argon-hashed.txt
