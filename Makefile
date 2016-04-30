###########################################################
#
# Simple Makefile for Operating Systems Project 5
#
#
###########################################################
.SUFFIXES: .h .c .cpp .l .o

MPI = mpic++
RM = /bin/rm
RMOPTS = -f

lamport: lamport.o
	$(MPI) lamport.o -o lamport -std=gnu++11

lamport.o: lamport.cpp
	 $(MPI) -c lamport.cpp -std=gnu++11

clean:
	$(RM) $(RMOPTS) *.o *~ core lamport
