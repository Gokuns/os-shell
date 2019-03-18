/*
* shelldon interface program

KUSIS ID: PARTNER NAME:
KUSIS ID: PARTNER NAME:

*/

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>

#define MAX_LINE       80 /* 80 chars per line, per command, should be enough. */

int parseCommand(char inputBuffer[], char *args[],int *background, char* file[], int* redir , int* comm_count);
int executeCommand(char *args[], char* file[],int redr, int backg); //char** hist);
int main(void)
{
  char inputBuffer[MAX_LINE]; 	        /* buffer to hold the command entered */
  int background;             	        /* equals 1 if a command is followed by '&' */
  char *args[MAX_LINE/2 + 1];	        /* command line (of 80) has max of 40 arguments */
  pid_t child;            		/* process id of the child process */
  int status;           		/* result from execv system call*/
  int shouldrun = 1;
  int i, upper;
  int redir;  //redirection flag
  char *file[MAX_LINE/2 + 1];	 //output filename
  //char *history[10];
  int command_count =-1;

  while (shouldrun){            		/* Program terminates normally inside setup */
    background = 0;
    redir =0;

    shouldrun = parseCommand(inputBuffer,args,&background, file, &redir, &command_count);       /* get next command */

    if (strncmp(inputBuffer, "exit", 4) == 0)
    shouldrun = 0;     /* Exiting from shelldon*/

    if (shouldrun) {
      /*
      After reading user input, the steps are
      (1) Fork a child process using fork()
      (2) the child process will invoke execv()
      (3) if command included &, parent will invoke wait()
      */
      executeCommand(args, file, redir, background);//, history);
      printf("%d\n", command_count );
      // if (command_count<10){
      //   for(int i =0; i<command_count; i++){
      //     printf("%s\n", history[i]);
      //   }
      // }else{
      //   for(int i =0; i<10; i++){
      //     printf("%s\n", history[i]);
      //
      //   }
      // }



    }
  }
  return 0;
}

/**
* The parseCommand function below will not return any value, but it will just: read
* in the next command line; separate it into distinct arguments (using blanks as
* delimiters), and set the args array entries to point to the beginning of what
* will become null-terminated, C-style strings.
*/

int parseCommand(char inputBuffer[], char *args[],int *background, char* file[], int* redir, int* comm_count )
{
  int length,		/* # of characters in the command line */
  i,		/* loop index for accessing inputBuffer array */
  start,		/* index where beginning of next command parameter is */
  ct,	        /* index of where to place the next parameter into args[] */
  command_number;	/* index of requested command number */

  ct = 0;
  int ct_2 =0;
  /* read what the user enters on the command line */
  do {
    printf("shelldon>");
    fflush(stdout);
    length = read(STDIN_FILENO,inputBuffer,MAX_LINE);
  }
  while (inputBuffer[0] == '\n'); /* swallow newline characters */
  printf("%s\n", inputBuffer);
  /**
  *  0 is the system predefined file descriptor for stdin (standard input),
  *  which is the user's screen in this case. inputBuffer by itself is the
  *  same as &inputBuffer[0], i.e. the starting address of where to store
  *  the command that is read, and length holds the number of characters
  *  read in. inputBuffer is not a null terminated C-string.
  */
  start = -1;
  if (length == 0)
  exit(0);            /* ^d was entered, end of user command stream */

  /**
  * the <control><d> signal interrupted the read system call
  * if the process is in the read() system call, read returns -1
  * However, if this occurs, errno is set to EINTR. We can check this  value
  * and disregard the -1 value
  */

  if ( (length < 0) && (errno != EINTR) ) {
    perror("error reading the command");
    exit(-1);           /* terminate with error code of -1 */
  }

  /**
  * Parse the contents of inputBuffer
  */

  for (i=0;i<length;i++) {
    /* examine every character in the inputBuffer */

    switch (inputBuffer[i]){
      case ' ':
      case '\t' :               /* argument separators */
      if(start != -1){
        if(*redir!=1 && *redir!=2){
          args[ct] = &inputBuffer[start];    /* set up pointer */
          ct++;
        }else{
          file[ct_2]=&inputBuffer[start];
          ct_2++;
        }
      }
      inputBuffer[i] = '\0'; /* add a null char; make a C string */
      start = -1;
      break;

      case '\n':                 /* should be the final char examined */
      if (start != -1){
        if(*redir!=1 && *redir!=2){
          args[ct] = &inputBuffer[start];
          ct++;
        }else{
          file[ct_2]=&inputBuffer[start];
          ct_2++;
        }
      }
      inputBuffer[i] = '\0';
      args[ct] = NULL; /* no more arguments to this command */
      break;

      default :             /* some other character */
      if (start == -1)
      start = i;
      if (inputBuffer[i] == '&') {
        *background  = 1;
        inputBuffer[i-1] = '\0';
      }
      if(inputBuffer[i] == '>'){
        if(i!=(length-1) && inputBuffer[i+1]=='>'){
          *redir=1;
          i=i+1;
          continue;
        }else if(i!=(length-1) && inputBuffer[i+1]!='>'){
          *redir=2;
          continue;

        }

      }


    } /* end of switch */
  }    /* end of for */

  /**
  * If we get &, don't enter it in the args array
  */

  if (*background)
  args[--ct] = NULL;

  args[ct] = NULL; /* just in case the input line was > 80 */
  *comm_count = *comm_count+1;
  printf("comm_count is %d\n", *comm_count%10);

  return 1;

} /* end of parseCommand routine */

int executeCommand(char *args[], char* file[],int redr, int backg){//, char* hist[]){


        pid_t pid;
        int out =0;
        int xf;
        pid=fork();
        if (pid == 0){ //child process
          // out=execvp(args[0], args);
          // printf("Redirected val is%d\n", redir);
          //
          // printf("Filename is %s\n", file[0] );
          // printf("Filename is %s\n", file[1] );
          // printf("Filename is %s\n", file[2] );
          mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
          if(redr ==2){
            xf=open(file[1],O_RDWR|O_TRUNC|O_CREAT, mode);
          }else if (redr ==1){
            xf=open(file[1], O_RDWR|O_APPEND|O_CREAT, mode);
          }
          dup2(xf,1);
          char* path = malloc(strlen("/bin/")+strlen(args[0]));
          strcpy(path,"/bin/" );
          strcat(path, args[0] );
          out=execv(path, args);
          if (out<0){
            execvp(args[0], args);
          }


          // history[c_count%10]=args;


          close(xf);
          exit(1);


        }else if(pid>0){

          if(backg != 1){
            // printf("Waitin for child to exit\n" );
            wait(NULL);

          }
          // printf("Child exited\n" );

        }

        // strncpy(hist[*comm_count], *args, MAX_LINE );
        // if(*comm_count==0){
        //   hist[0]=args[0];
        //
        // }else if (*comm_count==1){
        //   hist[1]=args[1];
        //
        // }else{
        //   hist[2]=args[0];
        //
        // }
        // // hist[0]=args[0];
        // printf("%s\n", hist[(*comm_count)%10]);
        return 1;
}
