//********************************************************************
//
// Will Taylor & James Stewart
// Operating Systems
// Project 5: Lamport and Vector Logical Clocks
// Due: April 29, 2016
// Language: C++ using the OpenMPI Library
// Instructor: Dr. Michael C. Scherger
//
//********************************************************************

These programs simulate both Lamport logical clocks and vector clocks across distributed systems.

Usage:
    You may compile both programs by using the provided make file.
    To run an application, use the terminal command "$mpirun -np [NUMBER_OF_PROCESSES] ./[EXECUTABLE_NAME]".

    Once you've begun running the application, you may enter in three different commands.
        - exec [PROCESS_NUMBER] This command instructs a process to perform a command and
            increment its own clock.
            
        - send [SENDING_PROCESSES] [RECEIVING_PROCESS] ["MESSAGE"] This command instructs the sending process to
            send MESSAGE to the receiving process. Note that you do need to include the quotation marks, but they will
            not be shown in the output.
        - end   This command cleanly terminates all processes and ends the simulation.   
            
Estimated time spent: about 6-8 hours each.