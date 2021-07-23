# Don't forget to generate `probes_desc.h` first

FLAGS=-O3 -mavx2 -mavx512vl -mavx512bw -mpopcnt -Wall -Wextra
CC=clang

OBJS =  \
 combinations.o \
 popcount256_16.o \
 checker_helper.o \
 checker.o \
 matverif.o \
 gray.o \
 prob_desc.o

matverif: $(OBJS)
	$(CC) -lpthread -o $@ $(OBJS)

matverif.o: matverif.c checker_helper.o prob_desc.o checker.o combinations.o gray.o
	$(CC) $(FLAGS) -o $@ -c $<

%.o: %.c %.h prob_desc.h
	$(CC) $(FLAGS) -o $@ -c $<

.PHONY: clean all

clean:
	rm -f $(OBJS) matverif *.sage.py parser.out parsetab.py

all: matverif

