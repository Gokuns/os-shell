/*
* shelldon interface program
KUSIS ID: 53940 PARTNER NAME: Asli Karahan
KUSIS ID: 54040 PARTNER NAME: Gökalp Ünsal
*/

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>

#define MAX_LINE       80 /* 80 chars per line, per command, should be enough. */

int parseCommand(char inputBuffer[], char *args[],int *background, char* file[], int* redir , int* comm_count, char *hist[], int* which_comm);
int executeCommand(char *args[], char* file[],int redr, int backg, char* hist[], int* comm_count, int which_comm); //char** hist);
int addToHistory(int ct, char *hist[], char context[]);
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
  char *history[11];
  int command_count =0;
  int which_command=-1;
  while (shouldrun){            		/* Program terminates normally inside setup */
    background = 0;
    redir =0;

    shouldrun = parseCommand(inputBuffer,args,&background, file, &redir, &command_count, history, &which_command);       /* get next command */

    if (strncmp(inputBuffer, "exit", 4) == 0)
    shouldrun = 0;     /* Exiting from shelldon*/
    if (shouldrun) {
      /*
      After reading user input, the steps are
      (1) Fork a child process using fork()
      (2) the child process will invoke execv()
      (3) if command included &, parent will invoke wait()
      */
      executeCommand(args, file, redir, background, history, &command_count, which_command);
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

int parseCommand(char inputBuffer[], char *args[],int *background, char* file[], int* redir, int* comm_count, char* hist[] , int* which_comm)
{
  int length,		/* # of characters in the command line */
  i,		/* loop index for accessing inputBuffer array */
  start,		/* index where beginning of next command parameter is */
  ct,	        /* index of where to place the next parameter into args[] */
  command_number;	/* index of requested command number */
  ct = 0;
  int ct_2 =0;
  /* read what the user enters on the command line */
  *comm_count = *comm_count+1; //increment the command count first
  int asd = *comm_count;

  if(*redir!=4 && *redir !=5){
    memset(inputBuffer, 0, MAX_LINE * sizeof(char));
    do {
      printf("shelldon>");
      fflush(stdout);
      length = read(STDIN_FILENO,inputBuffer,MAX_LINE);
    }
    while (inputBuffer[0] == '\n'); /* swallow newline characters */

  }else{
    length=strlen(inputBuffer);
  }

  addToHistory(asd, hist, inputBuffer);
  if(*redir==4) printf("i am here\n" );

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
      if(inputBuffer[i] == '>'){// Check this later
        if(i!=(length-1) && inputBuffer[i+1]=='>'){
          *redir=1;
          i=i+1;
          continue;
        }else if(i!=(length-1) && inputBuffer[i+1]!='>'){
          *redir=2;
          continue;

        }

      }else if(inputBuffer[i] == '!'){
        if(inputBuffer[i+1] == '!'){
          *redir=4;
          i=i+1;
          if(*comm_count!=1)
          addToHistory(asd, hist, hist[1]);
          *comm_count = *comm_count+1;

        }else{
          *redir=5;
          i=i+1;
          char num[4];
          int ptr =0;
          while(i!=(length-1)){
            num[ptr] = inputBuffer[i];
            printf("%c\n", inputBuffer[i]);
            ptr+=1;
            i=i+1;
          }
          int number = atoi(num);
          memset(num, 0, 4 * sizeof(char));
          *which_comm=number;

        }
      }else if(strncmp(inputBuffer, "history", 7) == 0){
        *redir=3;
      }else if(strncmp(inputBuffer, "codesearch", 10) == 0){
        *redir=6;
      }


    } /* end of switch */
  }    /* end of for */

  /**
  * If we get &, don't enter it in the args array
  */

  if (*background)
  args[--ct] = NULL;

  args[ct] = NULL; /* just in case the input line was > 80 */
  return 1;

} /* end of parseCommand routine */

int executeCommand(char *args[], char* file[],int redr, int backg, char *hist[], int *comm_count, int which_comm){


  pid_t pid;
  int out =0;
  int xf;

  pid=fork();
  if (pid == 0){ //child process


    int ct = *comm_count; // command counter
    if(redr==4){ //redr 4: executes the last command on the history
      if(*comm_count != 1){
        printf("Got so far\n" );
        parseCommand(hist[0], args, &backg, file, &redr, comm_count, hist, &which_comm);
      }else{
        printf("No history yet\n" );}
      }
      else if (redr == 5){
        if(which_comm>=*comm_count){
          *comm_count=ct+1;
          printf("You haven't provided that many commands\n");
        }  else if(which_comm<*comm_count-9){
          *comm_count=ct+1;
          printf("Command number not in recent history\n");
        }else{
          printf("Executing Command: %s from history\n", hist[*comm_count-which_comm]);
          parseCommand(hist[*comm_count-which_comm], args, &backg, file, &redr, comm_count, hist, &which_comm);
        }
      }

      if(redr ==3){
        int iter = ct;
        if(ct>10) iter=10;
        for(int i=1;i<=iter;i++){
          printf("%d- %s", ct-i, hist[i]);
        }
      }
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


      close(xf);
      exit(1);


    }else if(pid>0){
      //  if (strncmp(args[0], "cd", 2) == 0){
      //    chdir(args[1]);
      //  }
      if(backg != 1){
        wait(NULL);

      }

    }

    return 1;
  }

  int addToHistory(int ct, char *hist[], char context[]){
    if(ct<10){
      for(int i=ct;i>-1;i--){
        hist[i+1]=hist[i];
      }
    }else if (ct>=10){
      for(int i=10;i>-1;i--){
        hist[i+1]=hist[i];
      }
    }
    hist[0]= malloc(MAX_LINE * sizeof(char));
    strcat(hist[0], context);
  }
