#include <mpi.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <stdlib.h>

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
  int logicalClock;

  // Get rank and size of simulation
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);	/* get current process id */
  MPI_Comm_size (MPI_COMM_WORLD, &size);	/* get number of processes */

  // Print out the number of processes in simulation
  printf ("[0]: There are %d processes in the system\n", size);

  // Loop to get and handle user input
  while(true){
    string command;

    // Get command from user
    getline(std::cin, command);

    // Test for "end"
    if(command.compare("end") == 0){
      // Exit program
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
        //cout << *i << "\n";
        if(j == 0){
          if(tokens.at(j).compare("exec") == 0){
            // Exec command
            commandType = 0;
          } else if(tokens.at(j).compare("send") == 0){
            commandType = 1;
          }
        } else if(j == 1){
          destination = atoi(tokens.at(j).c_str());
        } else if(j == 2){
          messageDestination = atoi(tokens.at(j).c_str());
        } else if(j == 3){
          message = tokens.at(j);

          // Get rid of "s at front and end o string
          message = message.substr(1, message.length() - 1);
        }

        j++;
      }

      cout << "got tokens.\n";

      // Get destination to do command
      cout << "About to send...\n";
      MPI_Send(&commandType, 1, MPI_INT, destination, 0, MPI_COMM_WORLD);
      cout << "Sent\n";

      if(commandType == 1){
        // Message command

        // Tell messageDestination to listen for message from destination
        commandType = 2;
        MPI_Send(&commandType, 1, MPI_INT, messageDestination, 0, MPI_COMM_WORLD);

        // Tell destination what the messageDestination is
        cout << "Message dest: " << messageDestination << "\n";
        MPI_Send(&messageDestination, 1, MPI_INT, destination, 0, MPI_COMM_WORLD);

        // Send message to destination
        MPI_Send(message.c_str(), message.size() + 1, MPI_CHAR, destination, 0, MPI_COMM_WORLD);
        cout << "Sent message to destination\n";
      }

    }

  }
}

void worker(){
  int rank, size;
  int logicalClock;
  int command;

  logicalClock = 0;

  // Get rank and size of simulation
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);	/* get current process id */
  MPI_Comm_size (MPI_COMM_WORLD, &size);	/* get number of processes */

  while(true){
    MPI_Recv(&command, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("\t[%d]: Received command code %d\n", rank, command);
    if(command == 0){
      // We have an exec command
      logicalClock++;
      printf("\t[%d]: Execution Event: Logical Clock = %d\n", rank, logicalClock);
    } else if(command == 1){
      // We need to send a message


      // Get message destination ID from manager
      int messageDestination;

      MPI_Recv(&messageDestination, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      printf("\t[%d]: Received message destination: %d\n", rank, messageDestination);

      char *buf = new char[128];
      printf("\t[%d]: Listening to receive a message that we then need to send...\n", rank);
      MPI_Recv(&buf, 128, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      printf("\t[%d]: Received message from manager!\n", rank);


      // TODO: PROBLEM IS HERE
      string test = string(buf);
      cout << "Test: " << test << "\n";


      MPI_Send(&buf, 128, MPI_CHAR, messageDestination, 0, MPI_COMM_WORLD);
      printf("\t[%d]: Sent message to process %d\n", rank, messageDestination);
      //cout << "\t[" << rank << "]: I just got a command to send " << message << " to " << messageDestination << ".\n";

      // // Get the message itself from manager
      // char *buf = new char[128];
      // MPI_Recv(&message, 128, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);



      //printf("\t[%d]: I just got a command to send %s to %d.\n", rank, message, messageDestination);

      delete [] buf;
    } else if(command == 2){
      // We need to listen for a message and a clock value
      string message;
      char *buf = new char[128];
      printf("\t[%d]: Listening to receive a message...\n", rank);
      MPI_Recv(&buf, 128, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      printf("\t[%d]: Received message!\n", rank);

    }
  }
}
