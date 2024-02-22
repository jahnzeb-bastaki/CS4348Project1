#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

//Helper Functions
int loadData(char *dataFile, int memory[]){
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
  int PC, SP, IR, AC, X, Y;
}

void child(char *filename){
  int memory[2000];
  int len = loadData(filename, memory);
  

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
  child(file);

  fclose(file);
}

