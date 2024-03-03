/**
 * @file project1.c
 * @author Jahnzeb Bastaki (jnb200003@utdallas.edu)
 * NETID: jnb200003
 * @brief Project1 - Exploring Multiple Processes and IPC
 * CLASS: CS 4348 Section 004
 * PROFESSOR: Greg Ozbirn
 * @version 0.1
 * @date 2024-03-02
 * 
 */
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
// Struct Created to pass read/write instructions
struct read_write{
  char op;
  int address;
  int data;
};
//Helper Functions
int load_data(FILE *dataFile, int memory[]){
  int array_p = 0;
  while(!feof(dataFile)){
    char fileline[50];
    char temp_char[10];
    fgets(fileline, 50, dataFile);
    
    //Ignore newlines, spaces, and tabs
    if(fileline[0] == '\n' || fileline[0] == '\t' || fileline[0] == ' ')
      continue;
    
    //If line is new memory location
    int memFlag = 0;
    if('.' == fileline[0]){
      memFlag = 1;
    }

    //read integer
    int counter = 0;
    while(isdigit(fileline[counter+memFlag])){
      temp_char[counter] = fileline[counter+memFlag];
      counter++;
    }

    //to 'cap' off char
    temp_char[counter] = '\0';
    int temp_n = atoi(temp_char);
    // if memory location, set pointer to location, else load value into array
    if(memFlag){
      array_p = temp_n;
    }else{
      memory[array_p] = temp_n; 
      array_p++;
    }
 }
 return array_p;
}
// read data from memory
int read_from_mem(int addr){
  int val;
  struct read_write r1;
  r1.address = addr;
  r1.op = 'r';
  write(to_memory[1], &r1, sizeof(r1));
  read(to_cpu[0], &val, sizeof(int));
  return val;
}
// write data to memory
void write_to_memory(int data, int addr){
  //char input[6];
  //snprintf(input, sizeof(input), "w%d", addr);
  struct read_write w1;
  w1.op = 'w';
  w1.address = addr;
  w1.data = data;
  //write(to_memory[1], input, sizeof(input));
  write(to_memory[1], &w1, sizeof(w1));
}
// invalid access to memory - terminate program
void invalid_mem_access(bool mode){
  struct read_write invalid;
  invalid.op = 'i';
  if(mode){
    printf("Memory Violation: accessing system address 1000 in user mode\n");
  } else {
    printf("Memory Violation: accessing system address 999 in system mode\n");
  }
  write(to_memory[1], &invalid, sizeof(invalid));
  exit(1);
}
// program complete - terminate program
void end_program(){
  struct read_write end;
  end.op = 'e';
  write(to_memory[1], &end, sizeof(end));
  exit(EXIT_SUCCESS);
}
// not valid value of ir - terminate program
void invalid_ir(int val){
  struct read_write invalid;
  invalid.op = 'i';
  invalid.data = val;
  write(to_memory[1], &invalid, sizeof(invalid));
  EXIT_FAILURE;
}

//CPU
void parent(int t){
  srand(time(NULL));
  int user_stack = 999;
  int system_stack = 1999;
  int pc, sp, ir, ac, x, y, addr, port;
  bool jump;
  bool interrupt;
  bool mode;
  int timer, timer_count, intrpt_check;

  // initialize variables
  mode = true; // usr - true; kernel - false
  interrupt = false; // no interrupts
  pc = 0;
  sp = user_stack; 
  timer_count = 0;
  timer = t;

  int intrpt_pending = 0;
  while(true){

    if(timer != 0 && timer_count != 0){ // to prevent arithmetic error, timer count has to be above 0
      intrpt_check = timer_count % timer; // check to see if there is an instruction interrupt
      if(intrpt_check == 0 && interrupt)  // intrpt might happen while there is an intrpt happening, need to handle after interupt
        intrpt_pending++;

      // if there is not an interrupt happening, that means we need to check whether
      // there is either an instruction interrupt or if there are pending interrupts
      if(!interrupt && (intrpt_check == 0 || intrpt_pending)){
        mode = false; //kernel mode
        interrupt = true; // disable interrupts
        
        write_to_memory(sp, system_stack--);
        sp = system_stack;

        write_to_memory(pc, sp--);
        pc = 1000;

        if(intrpt_pending > 0) // dont want pending to go to negative
          intrpt_pending--;
      }
    }
    
    jump = false;
    ir = read_from_mem(pc);
    switch (ir){
    case 1: // Load Value
      ac = read_from_mem(++pc);
      break;
    case 2: // Load Address
      addr = read_from_mem(++pc);
      if(addr > user_stack && mode)
        invalid_mem_access(mode);
      if(addr < user_stack + 1 && !mode)
        invalid_mem_access(mode);
      ac = read_from_mem(addr);
      break;
    case 3: // Load Value at Address
      addr = read_from_mem(++pc);

      // Check invalid access
      if(addr > user_stack && mode)
        invalid_mem_access(mode);
      if(addr < user_stack + 1 && !mode)
        invalid_mem_access(mode);
      
      addr = read_from_mem(addr);

      // Check invalid access
      if(addr > user_stack && mode)
        invalid_mem_access(mode);
      if(addr < user_stack + 1 && !mode)
        invalid_mem_access(mode);
      
      ac = read_from_mem(addr);
      break;
    case 4: // Load Value at Address + X
      addr = read_from_mem(++pc) + x;

      // Check invalid access
      if(addr > user_stack && mode)
        invalid_mem_access(mode);
      if(addr < user_stack + 1 && !mode)
        invalid_mem_access(mode);

      ac = read_from_mem(addr);
      break;
    case 5: // Load Value at Address + Y
      addr = read_from_mem(++pc) + y;

      // Check invalid access
      if(addr > user_stack && mode)
        invalid_mem_access(mode);
      if(addr < user_stack + 1 && !mode)
        invalid_mem_access(mode);

      ac = read_from_mem(addr);
      break; 
    case 6: // Load from SP + X
      addr = sp + x + 1; // because stack pointer is pointing to an empty slot

      // Check invalid access
      if(addr > user_stack && mode)
        invalid_mem_access(mode);
      if(addr < user_stack + 1 && !mode)
        invalid_mem_access(mode);

      ac = read_from_mem(addr);
      break;
    case 7: // Store Address
      addr = read_from_mem(++pc);

      // Check invalid access
      if(addr > user_stack && mode)
        invalid_mem_access(mode);
      if(addr < user_stack + 1 && !mode)
        invalid_mem_access(mode);

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

      // Check invalid access
      if(addr > user_stack && mode)
        invalid_mem_access(mode);
      if(addr < user_stack + 1 && !mode)
        invalid_mem_access(mode);

      pc = addr;
      jump = true;
      break;
    case 21: // Jump to Address if AC == 0
      addr = read_from_mem(++pc);

      // Check invalid access
      if(addr > user_stack && mode)
        invalid_mem_access(mode);
      if(addr < user_stack + 1 && !mode)
        invalid_mem_access(mode);

      if(ac == 0){
        pc = addr;
        jump = true;
      }
      break;
    case 22: // Jump to Address if AC != 0
      addr = read_from_mem(++pc);

      // Check invalid access
      if(addr > user_stack && mode)
        invalid_mem_access(mode);
      if(addr < user_stack + 1 && !mode)
        invalid_mem_access(mode);

      if(ac != 0){
        pc = addr;
        jump = true;
      }
      break;
    case 23: //Push PC onto stack, jump to Address
      addr = read_from_mem(++pc);

      // Check invalid access
      if (addr > user_stack && mode)
        invalid_mem_access(mode);
      if(addr < user_stack + 1 && !mode)
        invalid_mem_access(mode);

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
    case 29:
      if(interrupt) // to make sure a system call not happening during a timer interrupt
        break;
      mode = false;
      interrupt = true; //disable interrupts
      jump = true;

      write_to_memory(sp, system_stack--);
      sp = system_stack;

      write_to_memory(++pc, sp--);
      pc = 1500;
      break;
    case 30:
      pc = read_from_mem(++sp);
      system_stack = ++sp;
      sp = read_from_mem(sp);

      mode = true; //user mode
      interrupt = false; //enable interrupts
      jump = true;
      break;
    case 50:
      end_program();
      break;
    default:
      invalid_ir(ir);
      break;
    }
    if(!jump) // When there has been a statement jump, skip incrementing pc
      pc++;
    timer_count++;
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

  while(true){
    struct read_write command;
    read(to_memory[0], &command, sizeof(command));
    char op = command.op;
    
    // op commands
    if(op == 'r'){
      write(to_cpu[1], &memory[command.address], sizeof(int));
    }else if(op == 'w'){
      int val = command.data;
      memory[command.address] = val;
    }else if(op == 'i'){
      exit(EXIT_FAILURE);
    }else if(op=='e'){
      exit(EXIT_SUCCESS);
    }else if(op == 'x'){
      printf("Invalid IR Value - IR Val:%d", command.data);
      exit(EXIT_FAILURE);
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

