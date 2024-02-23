#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <stdbool.h>

// 0 is reading, 1 is writing
int to_cpu[2], to_memory[2];

//Helper Functions
int load_data(FILE *dataFile, int memory[]){
  int array_p = 0;
  while(!feof(dataFile)){
    char fileline[50];
    char temp_char[10];
    fgets(fileline, 50, dataFile);

    int counter = 0;
    while(isdigit(fileline[counter])){
      temp_char[counter] = fileline[counter];
      counter++;
    }
    temp_char[counter] = '\0';
    int temp_n = atoi(temp_char);
    memory[array_p++] = temp_n;
 }
 return array_p;
}
int read_from_mem(int data){
  char input[6];
  int val;
  snprintf(input, sizeof(input), "r%d", data);
  write(to_memory[1], input, sizeof(input));
  read(to_cpu[0], &val, sizeof(int));
  return val;
}
void write_to_memory(int data, int addr){
  char input[6];
  snprintf(input, sizeof(input), "w%d", addr);
  write(to_memory[1], input, sizeof(input));
  write(to_memory[1], &data, sizeof(int));
}
void invalid(){
  printf("Invalid Memory Access");
  exit(1);
}
void end_program(){
  char input = 'e';
  write(to_memory[1], &input, sizeof(input));
  exit(EXIT_SUCCESS);
}

//CPU
void parent(){
  srand(time(NULL));
  int user_stack = 999;
  int system_stack = 1999;
  int usr_stack_pointer = user_stack;
  int system_stack_pointer = system_stack;
  bool mode = true; // usr = true; krnl = false

  int pc, sp, ir, ac, x, y, addr, port;
  pc = 0; sp = system_stack_pointer;

  while(true){
    ir = read_from_mem(pc);
    //printf("1st Statement - PC:%d  IR:%d  AC:%d  X:%d  Y:%d\n", pc, ir, ac, x, y);
    switch (ir){
    case 1:
      ac = read_from_mem(++pc);
      break;
    case 2:
      addr = read_from_mem(++pc);
      break;
    case 3:
      addr = read_from_mem(++pc);
      if(addr > 999 && mode)
        invalid();
      addr = read_from_mem(addr);
      if(addr > 999 && mode)
        invalid();
      ac = read_from_mem(addr);
      break;
    case 4:
      addr = read_from_mem(++pc) + x;
      if(addr > 999 && mode)
        invalid();
      ac = read_from_mem(addr);
      break;
    case 5:
      addr = read_from_mem(++pc) + y;
      if(addr > 999 && mode)
        invalid();
      ac = read_from_mem(addr);
      break; 
    case 6:
      addr = sp + x;
      if(addr > 999 && mode)
        invalid();
      ac = read_from_mem(addr);
      break;
    case 7:
      addr = read_from_mem(++pc);
      if(addr > 999 && mode)
        invalid();
      write_to_memory(ac, addr);
      break;
    case 8:
      ac = (rand() % 100) + 1;
      break;
    case 9:
      port = read_from_mem(++pc);
      if(port == 1){
        printf("%d", ac);
      } else {
        printf("%c", ac);
      }
      printf("\n");
      break;
    case 10:
      ac += x;
      break;
    case 11:
      ac += y;
      break;
    case 12:
      ac -= x;
      break;
    case 13:
      ac -= y;
      break;
    case 14:
      x = ac;
      break;
    case 15:
      ac = x;
      break;
    case 16:
      y = ac;
      break;
    case 17:
      ac = y;
      break;
    case 50:
      end_program();
      break;
    default:
      break;
    }
    pc++;
    //printf("2st Statement - PC:%d  IR:%d  AC:%d  X:%d  Y:%d\n\n", pc, ir, ac, x, y);
  }
}

void child(char *argv){
  //printf("PID: %d", getpid);
  FILE *file = fopen(argv, "r");
  // File not found
  if(!file){
    fprintf(stderr, "ERROR - file \"%s\" was not found", argv);
    exit(1);
  }

  int memory[2000];
  int len = load_data(file, memory);
  fclose(file);
  
  int usr_program = 0;
  int sys_program = 1000;
  
  while(true){
    char input[6];
    read(to_memory[0], input, sizeof(input));
    char op = input[0];
    for(int i = 0; i < 5; i++)
      input[i] = input[i + 1];
    if(op == 'r'){
      //printf("Memory:%d   Input:%d\n", memory[atoi(input)], atoi(input));
      write(to_cpu[1], &memory[atoi(input)], sizeof(int));
    }else if(op == 'w'){
      int val;
      read(to_memory[0], &val, sizeof(int));
      memory[atoi(input)] = val;
    }else if(op=='e'){
      //printf("Memory Process Exits\n");
      exit(EXIT_SUCCESS);
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

