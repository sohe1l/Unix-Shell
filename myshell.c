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

#define EXIT_KEY "exit"




#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"



void runCommand(char** av){

      printf(GRN "before if %d %d\n" RESET , getpid(), getppid());


    int status;
    int child_pid = fork();


    if(child_pid < 0){
      printf("Error while forking child process.\n");
    }else if(child_pid == 0){
      printf(RED "if1 %d %d\n" RESET , getpid(), getppid() );

      execvp(av[0], av);

      printf(RED "if2 %d %d\n" RESET, getpid(), getppid() );

    }else{

      printf(BLU "else1 %d %d\n" RESET, getpid(), getppid() );


      while(wait(&status) != child_pid);
      printf(BLU "else2 %d %d\n" RESET, getpid(), getppid() );

      /*
      pid_t tpid;
      do{
        tpid = wait(&status);
        if(tpid != child_pid) process_terminated(tpid);
      }while(tpid != child_pid);
      */
    }
}





int main(int* argc, char** argv)
{

  //get home path
  char *HOME = getenv("HOME");
  char input[BUFFERSIZE];
  char *myargv[256];
  int myargc = 0;

  printf(MAG "Starting %d %d\n" RESET, getpid(), getppid() );




/*

  char *args[1];
   args[0] = "ls\0";
   //args[1] = "\0";
   //args[1] = "/home/\0";

  execvp(args[0], args);

  return 0;
*/

while(1){

  printf(MAG "While %d %d\n" RESET, getpid(), getppid() );

  printf(PROMPT);

  fgets(input, BUFFERSIZE, stdin);


  char *res_search_input = strchr(input, '\n');
  if(res_search_input != NULL){
    *res_search_input = '\0';
  }

  if(input == NULL){
    printf("%s\n", "Goodbyezzzzz!");
  }

  char *token = strtok(input, " ");

   while( token != NULL ) {
      printf( " %s\n", token );
      
      char* temp = token;

      myargv[myargc] = temp;
      myargc = myargc + 1;

      token = strtok(NULL, " ");
   }

   // run 


   if( strcmp(myargv[0], EXIT_KEY) == 0){

    //int l = strlen("ls\0");

    //printf("Len %d \n", l);


    //strlen(myargv[0]);
    //printf( myargv[0] );
    //prtinf(strlen(myargv[0]));

    



    printf("%s\n", "Goodbye!");
    break;
        printf("%s\n", "AFTER BREAK!");

    return 0;
        printf("%s\n", "AFTER RETURN!");

   }else{
    printf(MAG "Before RUN %d %d\n" RESET, getpid(), getppid() );
    runCommand(myargv);
    printf(MAG "After Run %d %d\n" RESET, getpid(), getppid() );
   }

}
  return 0;
}
