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
#include <errno.h>

#include <sys/types.h>
#include <sys/wait.h>

//Max amount allowed to read from input
#define BUFFERSIZE 256
//Shell prompt
#define PROMPT "myShell >> "
//sizeof shell prompt
#define PROMPTSIZE sizeof(PROMPT)

#define EXIT_KEY "exit"
#define WHITESPACE " \t\r\n\v\f"



#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"


enum io_type {STANDARD, OUT_WRITE_FILE, OUT_APPEND_FILE, IN_FILE};


void runCommand(char** av, int is_bg,enum io_type ioType, char* ioFile){

  printf(GRN "before if %d %d\n" RESET , getpid(), getppid());

  int fp;
  if(ioType == OUT_WRITE_FILE){
    fp = open(ioFile, O_WRONLY | O_CREAT, 0666);
  }else if(ioType == OUT_APPEND_FILE){
    printf("appending \n");
    fp = open(ioFile,  O_APPEND | O_CREAT, 0666);
  }else if(ioType == IN_FILE){
    fp = open(ioFile, O_RDONLY);
  }

  int status;
  int fd[2];
  int child_pid = fork();

  

  if(child_pid < 0){
    printf("Error while forking child process.\n");
  }else if(child_pid == 0){ // child process

    if(is_bg){
      fclose(stdin);
      fopen("/dev/null/", "r");
    }

    pipe(fd);

    if(ioType == OUT_WRITE_FILE){
      //close(1);
      dup2(fp, 1);
    }else if(ioType == OUT_APPEND_FILE){
      //close(1);
      dup2(fp, 1);
    }else if(ioType == IN_FILE){
      //close(0);
      dup2(fp, 0);
    }
    
    printf(RED "if1 %d %d\n" RESET , getpid(), getppid() );

    if(execvp(av[0], av) == -1){
            printf(RED "Error occured: %s\n" RESET, strerror(errno) );

      exit(1);
    }else{
      exit(0);
    }

    
    printf(RED "if2 %d %d\n" RESET, getpid(), getppid() );

  }else{ // main process

    printf(BLU "else1 %d %d\n" RESET, getpid(), getppid() );

    if(!is_bg){
      while(wait(&status) != child_pid);
    }
    
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
  int myargc;
  int is_bg = 0;
  enum io_type ioType;
  char *ioFile;

  printf(MAG "Starting %d %d\n" RESET, getpid(), getppid() );





  // char *args[50];
  //  args[0] = "cat";
  //  args[1] = "readme.txt";
  //  args[2] = "README.md";
  //  args[3] = '\0';
  //  //args[1] = "/home/\0";

  // runCommand(args);

  // return 0;


while(1){

  myargc = 0;
  is_bg = 0;
  ioType = STANDARD;
  ioFile = "";

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

  char *token = strtok(input, WHITESPACE);

  while( token != NULL ) {

    if(strcmp(token, ">")==0){

      ioType = OUT_WRITE_FILE;

    }else if(strcmp(token, ">>")==0){

      ioType = OUT_APPEND_FILE;

    }else if(strcmp(token, "<")==0){

      ioType = IN_FILE;

    }else{

      if(ioType != STANDARD && strcmp(ioFile,"") == 0){
        ioFile = token;
      }else if(ioType != STANDARD && strcmp(ioFile,"") != 0){
        printf("Token is: %s   Length: %zu\n", token, strlen(token) );
        printf("ERROR INVALID PATTERN\n");
        exit(1); // should noy really end
      }else{
        myargv[myargc] = token;
        myargc = myargc + 1;
      }

    }


    token = strtok(NULL, WHITESPACE);
  }

   // add the end of arguments
    myargv[myargc] = '\0';
    myargc = myargc + 1;

   // run 


   if( strcmp(myargv[0], EXIT_KEY) == 0){
    printf("%s\n", "Goodbye!");
    return 0;
   }

  char *res_search_bg = strchr(myargv[myargc-2], '&');
  if(res_search_bg != NULL){
    *res_search_bg = '\0';
    is_bg = 1;
  }



    printf(MAG "Before RUN %d %d\n" RESET, getpid(), getppid() );

    printf(MAG "Count  Run %d \n" RESET, myargc );

    runCommand(myargv, is_bg, ioType, ioFile);

    printf(MAG "After Run %d %d\n" RESET, getpid(), getppid() );
   

}
  return 0;
}
