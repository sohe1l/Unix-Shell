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
#define PROMPT "myShell %s >> "
//sizeof shell prompt
#define PROMPTSIZE sizeof(PROMPT)

#define EXIT_KEY "exit"
#define WHITESPACE " \t\r\n\v\f"

//Colors
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

enum io_type {STANDARD, OUT_WRITE_FILE, OUT_APPEND_FILE, IN_FILE};

char current_dir[BUFFERSIZE];
char *home_dir;
char *token;
char input[BUFFERSIZE];

// To replace the ~ with home directory (or opposite)
char* replaceStrAtBeg(char* str, char* needle, char* replace){

  char* str_res = strstr(str, needle);

  if(str_res == NULL ){ // not found
    return str;
  }else if( (str_res - str) != 0 ){ // not at the beggining
    return str;
  }

  char* res = malloc( strlen(str) + strlen(replace) - strlen(needle) + 1 );

  strcpy(res, replace);

  strcpy(res+strlen(replace), str+strlen(needle));

  return res;
}


int run_cmd(char** av, int is_bg,enum io_type ioType, char* ioFile, int is_pipe, int fd_input){

  int status;
  int fd_in[2], fd_out[2];
  pipe(fd_in);
  pipe(fd_out);
  int child_pid = fork();

  if(child_pid < 0){
    printf("Error while forking child process.\n");
  }else if(child_pid == 0){ // child process


    if(fd_input != 0){
      while((dup2(fd_in[0], 0)==-1) && (errno==EINTR)){}
      close(fd_in[1]);
      close(fd_in[0]);      
    }

    if(is_pipe){
      while((dup2(fd_out[1], 1)==-1) && (errno==EINTR)){}
      close(fd_out[1]);
      close(fd_out[0]);
    }

    // if(is_bg){
    //   fclose(stdin);
    //   fopen("/dev/null/", "r");
    // }

    int fp;
    if(ioType == OUT_WRITE_FILE){
      fp = open(ioFile, O_TRUNC | O_WRONLY | O_CREAT, 0666);
      close(1);

    }else if(ioType == OUT_APPEND_FILE){
      fp = open(ioFile,  O_APPEND | O_WRONLY | O_CREAT, 0666);
      close(1);

    }else if(ioType == IN_FILE){
      fp = open(ioFile, O_RDONLY);
      close(0);
    }

    if(ioType != STANDARD){
      dup(fp);
    }

    if(execvp(av[0], av) == -1){
      if(fp) close(fp);
      exit(1);
    }else{
      if(fp) close(fp);
      exit(0);
    }
  }else{ // main process
    
    if(fd_input != 0){
      close(fd_in[0]);
      char buffer[BUFFERSIZE];
      int res_file_read;
      while(res_file_read = read(fd_input, buffer, BUFFERSIZE)){
        write(fd_in[1], buffer, res_file_read);
      }
      close(fd_in[1]);
    }


    if(is_pipe) close(fd_out[1]);

    if(!is_bg){
      while(wait(&status) != child_pid);
    }
    
    if(is_pipe) return fd_out[0];
    else return 0;
  }
}

void run_change_dir(char *dir){
  if(!dir) return;

  char *home_dir_with_slash = malloc(strlen(home_dir) +2);
  strcpy(home_dir_with_slash, home_dir);
  strcat(home_dir_with_slash, "/");

  char *chdir_path = replaceStrAtBeg(dir, "~", home_dir);

  if(chdir(chdir_path) != 0){
    perror("Failed changing directory"); 
  }
  getcwd(current_dir, BUFFERSIZE);
}


void print_prompt(){
  char *result = replaceStrAtBeg(current_dir, home_dir, "~");

  if(strcmp(result, "~")==0){
    result = "~/";
  }

  printf(GRN PROMPT RESET, result);
}



char* readTokens(){

  fgets(input, BUFFERSIZE, stdin);

  // replace new line with \0
  char *res_search_input = strchr(input, '\n');
  if(res_search_input != NULL){
    *res_search_input = '\0'; 
  }

  if(input == NULL){
    printf("%s\n", "Error reading input!");
  }

  token = strtok(input, WHITESPACE);
}



int main(int* argc, char** argv)
{

  //get home path
  char *myargv[256];
  int myargc;
  int is_bg = 0;
  enum io_type ioType;
  char *ioFile;

  home_dir = getenv("HOME");
  getcwd(current_dir, BUFFERSIZE);


  while(1){

    myargc = 0;
    is_bg = 0;
    ioType = STANDARD;
    ioFile = "";
    int isPipe = 0;
    int pipe_res_fd = 0;

    print_prompt();

    // read tokens into token variable
    readTokens();

    while( token != NULL ) {

      // printf("TOKEN: %s\n", token);

      if(strcmp(token, ">")==0){

        ioType = OUT_WRITE_FILE;

      }else if(strcmp(token, ">>")==0){

        ioType = OUT_APPEND_FILE;

      }else if(strcmp(token, "<")==0){

        ioType = IN_FILE;

      }else if(strcmp(token, "|")==0){ 

        // reaching a pipe character.. run command and continoue for next command
        myargv[myargc] = '\0';
        myargc = myargc + 1;

        //pipe_res_fd = run_pipe(myargv, pipe_res_fd);
        pipe_res_fd = run_cmd(myargv, is_bg, ioType, ioFile, 1, pipe_res_fd);

        myargc = 0; //clear args

      }else{
        if(ioType != STANDARD && strcmp(ioFile,"") == 0){
          ioFile = token;
        }else if(ioType != STANDARD && strcmp(ioFile,"") != 0){
          //printf("Token is: %s   Length: %zu\n", token, strlen(token) );
          printf("Error. Invalid usage for redirectors.\n");
          break; // move on
        }else{
          myargv[myargc] = malloc(strlen(token) + 1);
          strcpy(myargv[myargc], token);
          myargc = myargc + 1;
        }

      }


      token = strtok(NULL, WHITESPACE);
    }

    // at this point all the tokens are processed (and checked for any pipes or redirects)
    if(myargc != 0){
      // add the end of arguments
      myargv[myargc] = '\0';
      myargc = myargc + 1;


      if( strcmp(myargv[0], EXIT_KEY) == 0){
        printf(RED "%s\n" RESET , "Goodbye!");
        return 0;
      }

      if( strcmp(myargv[0], "pwd") == 0){
        printf("%s\n", current_dir);
        continue;
      }

      if( strcmp(myargv[0], "cd") == 0){
        run_change_dir(myargv[1]);
        continue;
      }

      char *res_search_bg = strchr(myargv[myargc-2], '&');
      if(res_search_bg != NULL){
        *res_search_bg = '\0';
        is_bg = 1;
      }

      if(pipe_res_fd != 0){

        // pipe_res_fd = run_pipe(myargv, pipe_res_fd);
        pipe_res_fd = run_cmd(myargv, is_bg, ioType, ioFile, 1, pipe_res_fd);

        char buffer[10];
        int res_file_read;
        while(res_file_read = read(pipe_res_fd, buffer, 10)){
          write(1, buffer, res_file_read);
        }

      }else{
        // runCommand(myargv, is_bg, ioType, ioFile);
        run_cmd(myargv, is_bg, ioType, ioFile, 0, 0);

      }
    }

  }
  return 0;
}