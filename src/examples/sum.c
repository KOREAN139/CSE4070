#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>

int main(int argc, char **argv){
  /* When number of given parameter is not
	 appropriate. */
  if(argc != 5) return EXIT_FAILURE;

  int a, b, c, d;
  a = atoi(argv[1]); b = atoi(argv[2]);
  c = atoi(argv[3]); d = atoi(argv[4]);

  printf("%d %d\n", fib(a), sumFour(a, b, c, d));

  return EXIT_SUCCESS;
}
