#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
  int fds[2] = {0, 0};
  pipe(fds);
  printf("read: %d write: %d\n", fds[0], fds[1]);

  if(fork()) { //parent
    char buffer[100];
    printf("Parent: Enter line: ");
    fgets(buffer, 100, stdin);

    //write to pipe
    write(fds[1], buffer, strlen(buffer)+1);
    wait(NULL);

  } else { //child
    char recieve[100];
    read(fds[0], recieve, 100);
    printf("Child got: %s\n", recieve);

  }
}
