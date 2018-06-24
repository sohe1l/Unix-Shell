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


char* replaceHomeDir(char* path){
  char* str_res = strstr(path, home_dir);

  if(str_res == NULL ){ // not found
    return path;
  }else if( (str_res - path) != 0 ){ // not at the beggining
    return path;
  }

  char* res = malloc( strlen(path) - strlen(home_dir) + strlen("~") + 1 );

  res[0] = '~';

  strcpy(res+1, path+strlen(home_dir));

  return res;
}


char* getFullPath(char* path){

  char* str_res = strstr(path, "~");

  if(str_res == NULL ){ // not found
    return path;
  }else if( (str_res - path) != 0 ){ // not at the beggining
    return path;
  }

  char* res = malloc( strlen(path) + strlen(home_dir) - strlen("~") + 1 );

  res[0] = '~';

  strcpy(res, home_dir);

  strcpy(res+strlen(home_dir), path+strlen("~"));

  return res;

}



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





void runCommand(char** av, int is_bg,enum io_type ioType, char* ioFile){

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

    int fp;
    if(ioType == OUT_WRITE_FILE){
      fp = open(ioFile, O_TRUNC | O_WRONLY | O_CREAT, 0666);
    }else if(ioType == OUT_APPEND_FILE){
      printf("appending \n");
      fp = open(ioFile,  O_APPEND | O_WRONLY | O_CREAT, 0666);
    }else if(ioType == IN_FILE){
      fp = open(ioFile, O_RDONLY);
    }


    if(ioType == OUT_WRITE_FILE){
      close(1);
      dup(fp);
      //dup2(fp, 1);
    }else if(ioType == OUT_APPEND_FILE){
      close(1);
      dup(fp);
      //dup2(fp, 1);
    }else if(ioType == IN_FILE){
      close(0);
      dup(fp);
      //dup2(fp, 0);
    }
    

    if(execvp(av[0], av) == -1){
      if(fp) close(fp);
      exit(1);
    }else{
      if(fp) close(fp);
      exit(0);
    }

    

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






int runPipe(char** av){

  int status;
  int fd[2];
  pipe(fd);
  int child_pid = fork();
  
  if(child_pid < 0){
    printf("Error while forking child process.\n");
  }else if(child_pid == 0){ // child process

    while((dup2(fd[1], 1)==-1) && (errno==EINTR)){}

    //dup2(fd[1], STDOUT_FILENO);
    close(fd[1]);
    close(fd[0]);
    

    execvp(av[0], av);
    exit(1);

    

  }else{ // main process
    close(fd[1]);

    while(wait(&status) != child_pid);
    return fd[0];
  }

  
}








int runPipe2(char** av, int fd_input){

  int status;
  int fd_in[2], fd_out[2];
  pipe(fd_in);
  pipe(fd_out);
  int child_pid = fork();
  
  if(child_pid < 0){
    printf("Error while forking child process.\n");
  }else if(child_pid == 0){ // child process

    while((dup2(fd_in[0], 0)==-1) && (errno==EINTR)){}
    close(fd_in[1]);
    close(fd_in[0]);

    while((dup2(fd_out[1], 1)==-1) && (errno==EINTR)){}
    close(fd_out[1]);
    close(fd_out[0]);
    
    execvp(av[0], av);
    exit(1);

  }else{ // main process
    close(fd_in[0]);
    char buffer[256];
    int res_file_read;
    while(res_file_read = read(fd_input, buffer, 256)){
      write(fd_in[1], buffer, res_file_read);
    }
    close(fd_in[1]);

    close(fd_out[1]);
    while(wait(&status) != child_pid);
    return fd_out[0];
  }

  
}

void run_change_dir(char *dir){

  if(!dir) return;

  char *home_dir_with_slash = malloc(strlen(home_dir) +2);
  strcpy(home_dir_with_slash, home_dir);
  strcat(home_dir_with_slash, "/");


  // char *chdir_path = getFullPath(dir);
  
  char *chdir_path = replaceStrAtBeg(dir, "~", home_dir);






  if(chdir(chdir_path) != 0){
    perror("Failed changing directory"); 
  }
  getcwd(current_dir, BUFFERSIZE);
}


void print_prompt(){
  
  //char *result = replaceHomeDir(current_dir);

  char *result = replaceStrAtBeg(current_dir, home_dir, "~");


  if(strcmp(result, "~")==0){
    result = "~/";
  }

  printf(GRN PROMPT RESET, result);

}

int main(int* argc, char** argv)
{

  //get home path
  char input[BUFFERSIZE];
  int input_c = 0;
  char *myargv[256];
  int myargc;
  int is_bg = 0;
  enum io_type ioType;
  char *ioFile;

  // get home dir and remove last slash if any
  home_dir = getenv("HOME");
  // if(home_dir[ strlen(home_dir) - 1 ] == '/'){
  //   home_dir[ strlen(home_dir) - 1 ] == '\0';
  //   printf("UPDATE HDRI\n");
  // }

  printf("HDIR %s\n", home_dir);

  getcwd(current_dir, BUFFERSIZE);


while(1){

  myargc = 0;
  is_bg = 0;
  ioType = STANDARD;
  ioFile = "";
  int isPipe = 0;
  int pipe_res_fd = 0;

  print_prompt();

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

    printf("TOKEN: %s\n", token);

    if(strcmp(token, ">")==0){

      ioType = OUT_WRITE_FILE;

    }else if(strcmp(token, ">>")==0){

      ioType = OUT_APPEND_FILE;

    }else if(strcmp(token, "<")==0){

      ioType = IN_FILE;

    }else if(strcmp(token, "|")==0){

      printf(MAG "Inside pipe %d %d\n" RESET, getpid(), getppid() );

      myargv[myargc] = '\0';
      myargc = myargc + 1;

      if(pipe_res_fd == 0 ){
        printf(MAG "Inside pipe if1 %d %d\n" RESET, getpid(), getppid());

        pipe_res_fd = runPipe(myargv);
      }else{
        printf(MAG "Inside pipe if2 %d %d\n" RESET, getpid(), getppid());
        pipe_res_fd = runPipe2(myargv, pipe_res_fd);
      }


      //clear args
      myargc = 0;



      // printf("AFTER THIS \n");
      // char buffer[10];
      // int res_file_read;
      // while(res_file_read = read(fd, buffer, 10)){
      //   printf("%s\n", "while");
      //   write(1, buffer, res_file_read);
      // }
      // printf("AFTER THIS \n");


  // char *ag[50];
  //  ag[0] = "grep";
  //  ag[1] = "txt";
  //  ag[2] = '\0';

  // int fd4 = runPipe2(ag, fd);

    // char buffer[10];
    // int res_file_read;
    // while(res_file_read = read(fd4, buffer, 10)){
    //   write(1, buffer, res_file_read);
    // }

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



   if( strcmp(myargv[0], EXIT_KEY) == 0){
    printf("%s\n", "Goodbye!");
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


      pipe_res_fd = runPipe2(myargv, pipe_res_fd);
      
      char buffer[10];
      int res_file_read;
      while(res_file_read = read(pipe_res_fd, buffer, 10)){
        write(1, buffer, res_file_read);
      }

    }else{
      
      runCommand(myargv, is_bg, ioType, ioFile);

    }
    


}
  return 0;
}
