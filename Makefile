#
#    Makefile for ysh
#

TARGET	= ysh
MAKE	:= make
CC	:= cc
RM	:= rm
CFLAGS	:= -O2 -g -Wall
DEFLAGS	:=
CMDLINE	:= 0
export

all clean:
	@$(MAKE) -C ./src	$@

.PHONY: all	\
	clean
