
.SUFFIXES:
.SUFFIXES: .o .c

CC	= gcc -O3 -Wall

PROGS	= example_code_1 example_code_2

#UDUNITS2_BASE_DIR = /usr/local
UDUNITS2_BASE_DIR = /usr
UDUNITS2_INC_DIR  = $(UDUNITS2_BASE_DIR)/include
UDUNITS2_LIB_DIR  = $(UDUNITS2_BASE_DIR)/lib

################################################################################

.c.o:
	$(CC) -c -I$(UDUNITS2_INC_DIR) $< 


################################################################################

TARGET: $(PROGS)

example_code_1: example_code_1.o calcalcs.o utCalendar2_cal.o
	$(CC) -o example_code_1 example_code_1.o calcalcs.o \
		utCalendar2_cal.o -L$(UDUNITS2_LIB_DIR) -ludunits2

example_code_2: example_code_2.o calcalcs.o 
	$(CC) -o example_code_2 example_code_2.o calcalcs.o 

tarfile:
	cd .. ; tar cvf calcalcs_1.0.tar 	\
		calcalcs_1.0/calcalcs.c 	\
		calcalcs_1.0/calcalcs.h 	\
		calcalcs_1.0/utCalendar2_cal.c 	\
		calcalcs_1.0/example_code_1.c 	\
		calcalcs_1.0/example_code_2.c 	\
		calcalcs_1.0/utCalendar2_cal.h 	\
		calcalcs_1.0/README 		\
		calcalcs_1.0/LICENSE 		\
		calcalcs_1.0/Makefile

clean:
	-rm *.o $(PROGS)


