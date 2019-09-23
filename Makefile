# Don't forget to generate `probes_desc.h` first

FLAGS=-O3 -mavx2 -mavx512vl -mavx512bw -mpopcnt -Wall -Wextra
CC=clang

OBJS =  \
 combinations.o \
 popcount256_16.o \
 checker_helper.o \
 checker.o \
 binverif.o \
 prob_desc.o

binverif: $(OBJS)
	$(CC) -lpthread -o $@ $(OBJS)

binverif.o: binverif.c checker_helper.o prob_desc.o checker.o combinations.o
	$(CC) $(FLAGS) -o $@ -c $<

%.o: %.c %.h prob_desc.h
	$(CC) $(FLAGS) -o $@ -c $<

.PHONY: clean all

clean:
	rm -f $(OBJS) binverif *.sage.py

all: binverif

