#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

//CPU
void parent(){
  int PC, SP, IR, AC, X, Y;
}

void child(char *filename){
  int memory[2000];

}



int main(int argc, char *argv[]) {
  // Args not found
  if (argc < 3) {
    fprintf(stderr, "ERROR - Usage: %s <filename> <interrupt>\n", argv[0]);
    exit(1);
  }
  FILE *file = fopen(argv[1], "r");
  // File not found
  if(!file){
    fprintf(stderr, "ERROR - file \"%s\" was not found", argv[1]);
    exit(1);
  }
  
}

