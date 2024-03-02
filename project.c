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
    //printf("%s", fileline);
    if(fileline[0] == '\n' || fileline[0] == '\t' || fileline[0] == ' ')
      continue;
    

    int memFlag = 0;
    if('.' == fileline[0]){
      memFlag = 1;
    }

    int counter = 0;
    while(isdigit(fileline[counter+memFlag])){
      temp_char[counter] = fileline[counter+memFlag];
      counter++;
    }

    temp_char[counter] = '\0';
    int temp_n = atoi(temp_char);
    if(memFlag){
      array_p = temp_n;
      //printf(" -- Memory pointer now at: %d\n", array_p);
    }else{
      memory[array_p] = temp_n; 
      //printf("Memory Address:%d Value:%d\n", array_p, memory[array_p]);
      array_p++;
    }
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
void invalid_mem_access(){
  char input = 'i';
  write(to_memory[1], &input, sizeof(input));
  printf("Invalid Memory Access");
  exit(1);
}
void end_program(){
  char input = 'e';
  write(to_memory[1], &input, sizeof(input));
  exit(EXIT_SUCCESS);
}
void invalid_ir(int val){
  char input[6];
  snprintf(input, sizeof(input), "i%d", val);
  write(to_memory[1], &input, sizeof(input));
  EXIT_FAILURE;
}

//CPU
void parent(int t){
  srand(time(NULL));
  int user_stack = 999;
  int system_stack = 1999;
  //int usr_stack_pointer = user_stack;
  int system_stack_pointer = system_stack;
  int pc, sp, ir, ac, x, y, addr, port;
  bool jump;
  bool interrupt;
  bool mode;
  int timer, timer_count;

  mode = true; // usr
  interrupt = false;
  pc = 0;
  sp = user_stack;
  timer_count = 0;

  while(true){
    

    

    timer_count++;
    jump = false;
    ir = read_from_mem(pc);
    //printf("1st Statement - PC:%d  IR:%d  AC:%d  X:%d  Y:%d Timer_Count:%d\n", pc, ir, ac, x, y, timer_count);
    switch (ir){
    case 1: // Load Value
      ac = read_from_mem(++pc);
      break;
    case 2: // Load Address
      addr = read_from_mem(++pc);
      break;
    case 3: // Load Value at Address
      addr = read_from_mem(++pc);
      if(addr > user_stack && mode)
        invalid_mem_access();
      addr = read_from_mem(addr);
      if(addr > user_stack && mode)
        invalid_mem_access();
      ac = read_from_mem(addr);
      break;
    case 4: // Load Value at Address + X
      addr = read_from_mem(++pc) + x;
      if(addr > user_stack && mode)
        invalid_mem_access();
      ac = read_from_mem(addr);
      break;
    case 5: // Load Value at Address + Y
      addr = read_from_mem(++pc) + y;
      if(addr > user_stack && mode)
        invalid_mem_access();
      ac = read_from_mem(addr);
      break; 
    case 6: // Load from SP + X
      addr = sp + x + 1; // because stack pointer is pointing to an empty slot
      if(addr > user_stack && mode)
        invalid_mem_access();
      ac = read_from_mem(addr);
      break;
    case 7: // Store Address
      addr = read_from_mem(++pc);
      if(addr > user_stack && mode)
        invalid_mem_access();
      write_to_memory(ac, addr);
      break;
    case 8: // Get Random Number
      ac = (rand() % 100) + 1;
      break;
    case 9: // Display Either Int or Char
      port = read_from_mem(++pc);
      if(port == 1){
        printf("%d", ac);
      } else {
        printf("%c", ac);
      }
      //printf("\n");
      break;
    case 10: // Add X to AC
      ac += x;
      break;
    case 11: // Add Y to AC
      ac += y;
      break;
    case 12: // Subtract X from AC
      ac -= x;
      break;
    case 13: // Subtract Y from AC
      ac -= y;
      break;
    case 14: // Copy Value in AC to X
      x = ac;
      break;
    case 15: // Copy Value in X to AC
      ac = x;
      break;
    case 16: // Copy Value in AC to Y
      y = ac;
      break;
    case 17: // Copy Value in Y to AC
      ac = y;
      break;
    case 18: // Copy Value in AC to SP
      sp = ac;
      break;
    case 19: // Copy Value in SP to AC
      ac = sp;
      break;
    case 20: // Jump to Address
      addr = read_from_mem(++pc);
      if(addr > user_stack && mode)
        invalid_mem_access();
      pc = addr;
      jump = true;
      break;
    case 21: // Jump to Address if AC == 0
      addr = read_from_mem(++pc);
      if(addr > user_stack && mode)
        invalid_mem_access();
      if(ac == 0){
        pc = addr;
        jump = true;
      }
      break;
    case 22: // Jump to Address if AC != 0
      addr = read_from_mem(++pc);
      if(addr > user_stack && mode)
        invalid_mem_access();
      if(ac != 0){
        pc = addr;
        jump = true;
      }
      break;
    case 23: //Push PC onto stack, jump to Address
      addr = read_from_mem(++pc);
      if (addr > user_stack && mode)
        invalid_mem_access();
      write_to_memory(++pc, sp--);
      pc = addr;
      jump = true;
      break;
    case 24: // Pop PC from stack, jump to PC
      pc = read_from_mem(++sp);
      jump = true;
      break;
    case 25: // X++
      x++;
      break;
    case 26: // X--
      x--;
      break;
    case 27: // Push AC onto Stack
      write_to_memory(ac, sp--);
      break;
    case 28:
      ac = read_from_mem(++sp);
      break;
    case 50:
      end_program();
      break;
    default:
      invalid_ir(ir);
      break;
    }
    if(!jump)
      pc++;
    //printf("2st Statement - PC:%d  IR:%d  AC:%d  X:%d  Y:%d\n\n", pc, ir, ac, x, y);
  }
}

void child(char *argv){
  FILE *file = fopen(argv, "r");
  // File not found
  if(!file){
    fprintf(stderr, "ERROR - file \"%s\" was not found", argv);
    exit(1);
  }

  int memory[2000];
  int len = load_data(file, memory);
  fclose(file);
  //for(int i = 0; i < 50; i++)
  //  printf("I:%d Value at Address:%d\n", i, memory[i]);
  
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
    }else if(op == 'i'){
      exit(EXIT_FAILURE);
    }else if(op=='e'){
      exit(EXIT_SUCCESS);
    }else if(op == 'x'){
      printf("Invalid IR Value - IR Val:%d", atoi(input));
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

  fork() ? parent(atoi(argv[2])) : child(argv[1]); 
}

