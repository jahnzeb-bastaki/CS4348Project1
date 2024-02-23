#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>

// 0 is reading, 1 is writing
int to_cpu[2], to_memory[2];

//Helper Functions
int loadData(FILE *dataFile, int memory[]){
  int array_p = 0;
  while(!feof(dataFile)){
    char fileline[50];
    char temp_char[10];
    int counter = 0;
    fgets(fileline, 50, dataFile);

    while(isdigit(fileline[counter])){
      temp_char[counter] = fileline[counter];
      counter++;
    }

    int temp_n = atoi(temp_char);
    memory[array_p++] = temp_n;
 }
 return array_p;
}

//CPU
void parent(){
  srand(time(0));
  int pc, sp, ir, ac, x, y;

  
}

void child(char *argv){
  FILE *file = fopen(argv, "r");
  // File not found
  if(!file){
    fprintf(stderr, "ERROR - file \"%s\" was not found", argv);
    exit(1);
  }

  int memory[2000];
  int len = loadData(file, memory);

  int usr_program = 0;
  int sys_program = 1000;

  char input[10];
  while(read(to_memory[0], input, sizeof(input))){
    char op = input[0];
    for(int i = 0; i < sizeof(input) - 1; i++)
      input[i] = input[i+1];
      
    if(op == "r"){
      write(to_cpu[1], memory[atoi(input)], sizeof(int));
    }else if(op == "w"){
      int val;
      read(to_memory[0], val, sizeof(int));
      memory[atoi(input)] = val;
    }else{
      printf("Memory Process Exits\n");
      exit(1);
    }
  }
}



int main(int argc, char *argv[]) {
  // Args not found
  if (argc < 3) {
    fprintf(stderr, "ERROR - Usage: %s <filename> <interrupt>\n", argv[0]);
    exit(1);
  }
  
  pipe(to_cpu); pipe(to_memory);

  fork() ? parent() : child(argv[1]); 
}

