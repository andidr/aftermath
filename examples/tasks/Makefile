AFTERMATH_SRC=../../src
CFLAGS=-std=c99 -Wall -I$(AFTERMATH_SRC) -O0 -g

all: tasks

tasks: $(AFTERMATH_SRC)/trace_file.h \
	$(AFTERMATH_SRC)/ansi_extras.h \
	$(AFTERMATH_SRC)/convert.h \
	$(AFTERMATH_SRC)/convert.c \
	$(AFTERMATH_SRC)/trace_file.c \
	tasks.c
	$(CC) $(CFLAGS) -o $@ $(filter %.c,$^)

clean:
	rm -f tasks
