/****************************************************************
 * Name        :                                                *
 * Class       : CSC 415                                        *
 * Date        :                                                *
 * Description :  Writting a simple bash shell program          *
 * 	          that will execute simple commands. The main   *
 *                goal of the assignment is working with        *
 *                fork, pipes and exec system calls.            *
 ****************************************************************/
//May add more includes
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//Max amount allowed to read from input
#define BUFFERSIZE 256
//Shell prompt
#define PROMPT "myShell >> "
//sizeof shell prompt
#define PROMPTSIZE sizeof(PROMPT)

int main(int* argc, char** argv)
{

  char input[BUFFERSIZE];
  char *myargv[256];
  int myargc = 0;



  printf(PROMPT);

  fgets(input, BUFFERSIZE, stdin);


  char *res_search_input = strchr(input, '\n');
  if(res_search_input != NULL){
    *res_search_input = '\0';
  }

  if(input == NULL){
    printf("%s\n", "Goodbye!");
  }

  char *token = strtok(input, " ");

   while( token != NULL ) {
      printf( " %s\n", token );
      
      char* temp = token;

      myargv[myargc] = temp;
      myargc = myargc + 1;

      token = strtok(NULL, " ");
   }

    
  return 0;
}
