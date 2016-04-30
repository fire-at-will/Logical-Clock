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
	$(MPI) lamport.o -o lamport

vector:	vector.o
	$(MPI) vector.o -o vector

lamport.o: lamport.cpp
	 $(MPI) -c lamport.cpp

vector.o: vector.cpp
	 $(MPI) -c vector.cpp

clean:
	$(RM) $(RMOPTS) *.o *~ core lamport
