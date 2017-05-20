#
# Copyright (C) Telecom ParisTech
# 
# This file must be used under the terms of the CeCILL. This source
# file is licensed as described in the file COPYING, which you should
# have received as part of this distribution. The terms are also
# available at:
# http://www.cecill.info/licences/Licence_CeCILL_V1.1-US.txt
#

SHELL		:= /bin/bash

CC		:= gcc
CFLAGS		:= -Wall -c
OPT		:= -O3
PYCFLAGS	:= -std=c99 -fPIC -DPYTHON_LIB $(shell python-config --cflags)
INCLUDES	:= -I.
LD		:= gcc
LDFLAGS		:=
PYLDFLAGS	:= -shared -fPIC
LIBS		:= -lm

OBJS		:= $(patsubst %.c,%.o,$(wildcard *.c))
EXECS		:= ta_acquisition ta test
PYSOS		:= des.so km.so
PYCS		:= $(patsubst %.py,%.pyc,$(wildcard *.py))
PYOBJS		:= $(patsubst %.c,PyC_%.o,$(wildcard *.c))
DATA		:= ta.dat ta.key

.PHONY: help all clean ultraclean check

define HELP_message
Type:
	make GOAL

Goals:
	help		print this message
	all		build everything
	ccheck		check the compliance of your code with specifications (C version)
	pycheck		check the compliance of your code with specifications (python version)
	clean		to clean
endef
export HELP_message

help:
	@echo "$$HELP_message"

all: $(EXECS) $(PYSOS)

ta_acquisition: ta_acquisition.o des.o rdtsc_timer.o utils.o p.o
	$(LD) $(LDFLAGS) $^ -o $@ $(LIBS)

ta: ta.o des.o km.o utils.o pcc.o
	$(LD) $(LDFLAGS) $^ -o $@ $(LIBS)

test: test.o rdtsc_timer.o utils.o p.o
	$(LD) $(LDFLAGS) $^ -o $@ $(LIBS)

Py_%.o PyC_%.o: CFLAGS += $(PYCFLAGS)

PyC_%.o: %.c
	$(CC) $(CFLAGS) $(OPT) $(INCLUDES) $< -o $@

p.o: p.c
	$(CC) $(CFLAGS) -O0 $(INCLUDES) $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(OPT) $(INCLUDES) $< -o $@

des.so: Py_des.o PyC_des.o PyC_utils.o
	$(LD) $(PYLDFLAGS) $^ -o $@ $(LIBS)

km.so: Py_km.o PyC_km.o PyC_utils.o PyC_des.o
	$(LD) $(PYLDFLAGS) $^ -o $@ $(LIBS)

ccheck: TA = ta
pycheck: TA = ta.py
ccheck: TASRC = ta.c
pycheck: TASRC = ta.py

ccheck pycheck: ta_acquisition $(TA)
	@echo "# Acquisition:" && \
	echo "./ta_acquisition 1000" && \
	k16=$$( ./ta_acquisition 1000 ) && \
	if [ -z "$$k16" ]; then \
		echo "***** FAILED (no output) *****" && \
		exit 1; \
	elif [[ ! "$$k16" =~ ^0x[0-9a-fA-F]{12}$$ ]]; then \
		echo "***** FAILED (invalid output $$k16) *****" && \
		exit 1; \
	fi && \
	echo "# Attack:" && \
	echo "./$(TA) ta.dat 1000" && \
	k16=$$( ./$(TA) ta.dat 1000 ) && \
	if [ -z "$$k16" ]; then \
		echo "***** FAILED (no output) *****" && \
		exit 1; \
	elif [[ ! "$$k16" =~ ^0x[0-9a-fA-F]{12}$$ ]]; then \
		echo "***** FAILED (invalid output $$k16) *****" && \
		exit 1; \
	fi && \
	echo "# Check OK, you can now push your work (if asked to, accept the merge):" && \
	echo "git add p.c $(TASRC)" && \
	echo "git commit -m 'My timing attack'" && \
	echo "git pull; git push"

clean:
	rm -f $(OBJS) $(EXECS) $(PYSOS) $(PYCS) $(PYOBJS) $(DATA)

