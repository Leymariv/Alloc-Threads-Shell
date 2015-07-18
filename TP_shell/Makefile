CC=gcc
CFLAGS+=-Wall -g -std=gnu99
LDLIBS+=-lreadline -lcurses

all: ensishell

ensishell: ensishell.o readcmd.o

clean:
	-$(RM) ensishell ensishell.o readcmd.o

test: ensishell
	expect ./testshell.expect ./ensishell
