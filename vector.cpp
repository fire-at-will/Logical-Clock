//********************************************************************
//
// Will Taylor & James Stewart
// Operating Systems
// Project 5: Lamport and Vector Logical Clocks
// Due: April 29, 2016
// Instructor: Dr. Michael C. Scherger
//
//********************************************************************

#include <mpi.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <stdlib.h>
#include <cstring>
#include <sstream>
#include <ctype.h>

using namespace std;

void manager();
void worker();

int main (int argc, char *argv[]){
  int rank, size;

  // Init MPI
  MPI_Init (&argc, &argv);	/* starts MPI */
  // Get rank and size of simulation
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);	/* get current process id */
  MPI_Comm_size (MPI_COMM_WORLD, &size);	/* get number of processes */

  if(rank == 0){
    // We are the manager
    manager();
  } else {
    // We are a worker process
    worker();
  }

  MPI_Finalize();
  return 0;
}

void manager(){
  int rank, size;

  // Get rank and size of simulation
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);	/* get current process id */
  MPI_Comm_size (MPI_COMM_WORLD, &size);	/* get number of processes */

  int vectorClock[size-1] = {};

  // Print out the number of processes in simulation
  printf ("[0]: There are %d processes in the system\n", size);

  // Loop to get and handle user input
  while(true){
    string command;
    int recv;

    // Get command from user
    getline(std::cin, command);


    // Test for "end"
    if(command.compare("end") == 0){
      // Exit program
      // Tell other processes to quit
      for(int i = 1; i < size; i++){
        MPI_Send("Q", 1, MPI_CHAR, i, 0, MPI_COMM_WORLD);
      }
      // Wait for other processes to finish
      printf("[%d]: Simulation ending\n", rank);

      MPI_Barrier(MPI_COMM_WORLD);

      return;
    } else {
      // Parse command
      string buf; // Have a buffer string
      stringstream ss(command); // Insert the string into a stream
      vector<string> tokens; // Create vector to hold our words
      while (ss >> buf){
          tokens.push_back(buf);
      }

      // Command Type 0 = Exec
      // Command Type 1 = Message
      int destination, j, messageDestination;
      string message;
      int commandType = 0;
      j = 0;
      for(vector<string>::iterator i = tokens.begin(); i != tokens.end(); i++){
        if(j == 0){
          if(tokens.at(j).compare("exec") == 0){
            // Exec command
            commandType = 0;
          } else if(tokens.at(j).compare("send") == 0){
            commandType = 1;
          }
        } else if(j == 1){
          destination = atoi(tokens.at(j).c_str());
          if(destination == 0){
            printf("ERROR: Manager cannot perform instructions. Assigning to process 1.\n");
            destination = 1;
          }
        } else if(j == 2){
          messageDestination = atoi(tokens.at(j).c_str());
        } else if(j == 3){
          message = tokens.at(j);
        } else {
          message += " ";
          message += string(tokens.at(j));
        }
        j++;
      }

      // Get rid of "s at front and end o string
      if(commandType == 1){
        message = message.substr(1, message.length() - 2);
      }

      if(commandType == 0){
        // We need to send an exec
        MPI_Send("EXEC", 4, MPI_CHAR, destination, 1, MPI_COMM_WORLD);
      } else if(commandType == 1){
        // We need to send a message
        // We want to pass the destination in the tag, but ensure that
        // no tag collisions occur.
        int tag = messageDestination + 3;
        MPI_Send(message.c_str(), message.size(), MPI_CHAR, destination, tag, MPI_COMM_WORLD);
      }

    }

    // Wait till command is done executing to execute next command
    MPI_Recv(&recv, sizeof(int), MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }
}

void worker(){
  int rank, size;
  bool done = false;

  MPI_Comm_size (MPI_COMM_WORLD, &size);	/* get number of processes */

  int vectorClock[size-1] = {};

  MPI_Comm_rank (MPI_COMM_WORLD, &rank);	/* get current process id */

  while(!done){
    char buf[128] = "";
    MPI_Status status;
    int count;

    MPI_Recv(&buf, 128, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

    string message = buf;
    int done = 0;

    // See what message type is
    if(status.MPI_TAG == 0){
      // Quit
      // Wait for other processes to finish
      MPI_Barrier(MPI_COMM_WORLD);

      ostringstream s;

      s << "\t[" << rank << "]: Vector Clock = [" << int(vectorClock[0]) << ", ";

      for (int jj = 1; jj < size-1; jj++) {
        if (jj == size-2) {
          s << " " << vectorClock[jj] << "]\n";
        } else {
          s << " " << vectorClock[jj] << ", ";
        }
      }

      cout << s.str();

      return;
    } else if(status.MPI_TAG == 1){
      // Exec command
      vectorClock[rank-1]++;

      printf("\t[%d]: Execution Event: Vector Clock = [%d,", rank, int(vectorClock[0]));
      for (int jj = 1; jj < size-1; jj++) {
        if (jj == size-2) {
          printf(" %d]\n", vectorClock[jj]);
        } else {
          printf(" %d,", vectorClock[jj]);
        }
      }

      MPI_Send(&done, sizeof(int), MPI_INT, 0, 0, MPI_COMM_WORLD);
    } else if(status.MPI_TAG == 2){
      // Receiving a message
      vectorClock[rank-1]++;

      MPI_Status stat;
      int senderRank = stat.MPI_SOURCE;

      int colonIndex = message.find(":") - 1;

      for (int jj = 0; jj < size - 1; jj++) {
        colonIndex = colonIndex + 2;
        string clockValString;
        if (isdigit(message.at(colonIndex+1))) {
          clockValString = message.substr(colonIndex, colonIndex+1);
        } else {
            clockValString = &message.at(colonIndex);
        }
        int clockVal = atoi(clockValString.c_str());

        if(clockVal > vectorClock[jj]){
          vectorClock[jj] = clockVal;
        }
      }

      message.resize(message.size() - ((size - 1) * 2));

      ostringstream s;
      s << "\t[" << rank << "]: Message Received from " << status.MPI_SOURCE << ": Message>" << message << "<: Vector clock = [" << vectorClock[0] << ",";
      for (int jj = 1; jj < size-1; jj++) {
        if (jj == size-2) {
          s << " " << vectorClock[jj] << "]\n";
        } else {
         s << " " << vectorClock[jj] << ",";
        }
      }
      cout << s.str();

      MPI_Send(&done, sizeof(int), MPI_INT, 0, 0, MPI_COMM_WORLD);
    } else {
      // Need to send a message to another process
      vectorClock[rank-1]++;

      ostringstream s;
      for (int jj = 0; jj < size-1; jj++) {
        s << ":" << vectorClock[jj];

      }
      s << ":";
      message.append(s.str());

      int destination = status.MPI_TAG - 3;
      MPI_Send(message.c_str(), message.size(), MPI_CHAR, destination, 2, MPI_COMM_WORLD);

      // Clear out outputstream
      s.str("");
      s.clear();

      s << "\t[" << rank << "]: Message Sent to: " << status.MPI_TAG - 3 << ": Message>" << string(buf) << "< Vector Clock = [" << vectorClock[0] << ",";

      for (int jj = 1; jj < size-1; jj++) {
        if (jj == size-2) {
          s << " " << vectorClock[jj] << "]\n";
        } else {
          s << " " << vectorClock[jj] << ",";
        }
      }
      cout << s.str();
    }
  }

}
