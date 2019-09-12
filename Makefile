# Don't forget to generate `probes_desc.h` first

FLAGS=-O3 -mavx2 -mpopcnt -Wall -Wextra
CC=clang

OBJS =  \
 combinations.o \
 popcount256_16.o \
 checker.o \
 prob_desc.o \
 check_security.o

check_sec: $(OBJS)
	$(CC) -o $@ $(OBJS)

check_security.o: check_security.c prob_desc.h checker.c
	$(CC) $(FLAGS) -o $@ -c $<

%.o: %.c %.h
	$(CC) $(FLAGS) -o $@ -c $<

.PHONY: clean all

clean:
	rm -f $(OBJS) check_sec gen_prob_desc.sage.py

all: check_sec

