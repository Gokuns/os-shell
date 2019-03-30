/*
* shelldon interface program
KUSIS ID: 53940 PARTNER NAME: Asli Karahan
KUSIS ID: 54040 PARTNER NAME: Gökalp Ünsal
*/

#include <wait.h>
#include <libgen.h>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_LINE       80 /* 80 chars per line, per command, should be enough. */

int parseCommand(char inputBuffer[], char *args[],int *background, char* file[], int* redir , int* comm_count, char *hist[], int* which_comm, int* histflag);
int executeCommand(char *args[], char* file[],int redr, int backg, char* hist[], int* comm_count, int which_comm, int histflag); //char** hist);
int addToHistory(int ct, char *hist[], char context[]);
int codeSearch(char *path, int mode, char *keyword);
int concatKeyword(char result[], char *args[]);
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
  int histflag;
  while (shouldrun){            		/* Program terminates normally inside setup */
    background = 0;
    redir =0;
    histflag=0;
    shouldrun = parseCommand(inputBuffer,args,&background, file, &redir, &command_count, history, &which_command, &histflag);       /* get next command */

    if (strncmp(inputBuffer, "exit", 4) == 0)
    shouldrun = 0;     /* Exiting from shelldon*/
    if (shouldrun) {
      /*
      After reading user input, the steps are
      (1) Fork a child process using fork()
      (2) the child process will invoke execv()
      (3) if command included &, parent will invoke wait()
      */
      executeCommand(args, file, redir, background, history, &command_count, which_command, histflag);
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

int parseCommand(char inputBuffer[], char *args[],int *background, char* file[], int* redir, int* comm_count, char* hist[] , int* which_comm, int* histflag)
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


  if(*histflag!=1 && *histflag !=2){
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
          *histflag=1;
          i=i+1;
          if(*comm_count!=1)
          addToHistory(asd, hist, hist[1]);
          *comm_count = *comm_count+1;

        }else{
          *histflag=2;
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
        *histflag=3;
      }else if(strncmp(inputBuffer, "codesearch", 10) == 0){
        *redir=6;
      }else if (strncmp(inputBuffer, "birdakika", 9) == 0){
        *redir =7;
      }else if(strncmp(inputBuffer, "snapshot", 8) == 0){
        *redir =8;
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

int executeCommand(char *args[], char* file[],int redr, int backg, char *hist[], int *comm_count, int which_comm, int histflag){


  pid_t pid;
  int out =0;
  int xf;

  pid=fork();
  if (pid == 0){ //child process


    int ct = *comm_count; // command counter
//======================================================================


/*
redr 4: executes the last command on the history
if its the first command, cannot does not execute.

*/

    if(histflag==1){
      if(*comm_count != 1){
        parseCommand(hist[0], args, &backg, file, &redr, comm_count, hist, &which_comm, &histflag);
      }else{
        printf("No history yet\n" );}
      }
//======================================================================

/*
redr 5: executes the desired command in history, if it is
in the range of commands.
*/
      else if (histflag == 2){
        if(which_comm>=*comm_count){
          *comm_count=ct+1;
          printf("You haven't provided that many commands\n");
        }  else if(which_comm<*comm_count-9){
          *comm_count=ct+1;
          printf("Command number not in recent history\n");
        }else{
          printf("Executing Command: %s\n", hist[*comm_count-which_comm]);
          parseCommand(hist[*comm_count-which_comm], args, &backg, file, &redr, comm_count, hist, &which_comm, &histflag);
        }
      }
//======================================================================

/*
Code Search Feature

*/

      if (redr == 6){
        char context[80];
        printf("%s\n",args[1] );
        if(strncmp(args[1],"-r",2)==0){

          int num= concatKeyword(context,args);
          codeSearch(".",2,context);

        }else if(strncmp(args[1],"-f",2)==0){
  printf("%s\n",args[3] );
          int num = concatKeyword(context,args);
  printf("%d\n",num );
          codeSearch(args[num+2],1,context);
       }else{
         //printf("%s\n",args[0] );
        int num= concatKeyword(context,args);
        codeSearch("", 0, context);

       }

      }
//======================================================================
    if(redr == 7){
      printf("trying to play %s at %s \n", args[2], args[1]);
      if(strlen(args[1])<4 || strlen(args[1])>5 || args[2]== NULL){
      printf("illegal arg\n" );
      exit(1);
    }
    char hour[] = "00";
    char min[] = "00";
    if(args[1][2]=='.' || args[1][2]==':' ){
    hour[0]=args[1][0];
    hour[1]=args[1][1];
    min[0]=args[1][3];
    min[1]=args[1][4];
    }

      FILE* shellptr = fopen("songplaying.sh", "w");
      if(shellptr==NULL)
      printf("error in opening the file\n");

      fprintf(shellptr, "#!/bin/bash\n/usr/bin/mpg321 --frames 2500 %s",args[2]);

      fclose(shellptr);
      char comm[255];

      strcpy(comm, min);
      strcat(comm, " ");
      strcat(comm, hour);
      strcat(comm, " * * * /bin/sh /home/asli/os_shell/songplaying.sh");
      // printf("Crontab enrty is %s\n", comm );
      // FILE* cronptr=fopen("/home/asli/var/spool/cron/crontabs/asli", "w");
      // fprintf(cronptr, "MAILTO = ""\n%s",comm);
      char crontabPath[200];
      strcpy(crontabPath,"(crontab -u asli -l ; echo \"");
      strcat(crontabPath,comm);
      strcat(crontabPath,"\") | crontab -u asli -");
      system(crontabPath);
 // fclose(cronptr);

    }
    if (redr == 8){
      printf("trying to get a snapshot\n" );
      printf("Arguments is %s\n",args[1] );
      if(args[1]==NULL){
        FILE* snapptr = fopen("history_snapshot.txt", "w");
        if(snapptr==NULL)
        printf("error in opening the file\n");
        char text[MAX_LINE*12];
        strcpy(text,"The call to snapshot produces the following output\n");
        printf("%s\n", text );
        int iter = *comm_count;
        if(*comm_count>10) iter=10;
        for(int i=1;i<=iter-1;i++){
          // printf("%d\n", i );
          //
          // char snum[5];
          // itoa(i, snum, 10);
          // strcat(text, snum);
          strcat(text, hist[i]);
        }

        fprintf(snapptr, "\n%s",text);
        fclose(snapptr);

      }else{
        char filename[MAX_LINE];
        strcpy(filename, args[1]);
        strcat(filename, ".txt");
        printf("%s\n", filename);

        FILE* snapptr = fopen(filename, "w");
        if(snapptr==NULL)
        printf("error in opening the file\n");
        char text[MAX_LINE*12];
        strcpy(text,"The call to snapshot produces the following output\n");
        printf("%s\n", text );
        int iter = *comm_count;
        if(*comm_count>10) iter=10;
        for(int i=1;i<=iter-1;i++){
          // printf("%d\n", i );
          //
          // char snum[5];
          // itoa(i, snum, 10);
          // strcat(text, snum);
          strcat(text, hist[i]);
        }

        fprintf(snapptr, "\n%s",text);
        fclose(snapptr);

      }

    }


/*
history command execution below

*/
      if(histflag ==3){
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
      // if (out<0){
      //   execvp(args[0], args);
      // }


      close(xf);
      exit(1);


    }else if(pid>0){
      // if (strncmp(args[0], "cd", 2) == 0){
      //    chdir(args[1]);
      //  }
      if(backg != 1){
        wait(NULL);

      }

    }

    return 1;
  }
//======================================================================

  int addToHistory(int ct, char *hist[], char context[])
  {
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

  int codeSearch(char *path, int mode, char *keyword)
  {

    int len;
    len = strlen(keyword);
    FILE  *fptr;
    DIR *d;
    struct dirent *dir;
    int index;
    char subbuff[len];
    memcpy( subbuff, &keyword[0], len);
    subbuff[len] = '\0';


    if(mode == 1)
    {

      char* ts1 = strdup(path);
     char* ts2 = strdup(path);
     printf("hidda\n");
     char* directory = dirname(ts1);
     char* filename = basename(ts2);
      d = opendir(directory);



         dir=readdir(d);

                 fptr = fopen( filename, "r");
                char line [ 500 ]; /* or other suitable maximum line size */
                index=0;
                 while ( fgets ( line, sizeof line, fptr ) != NULL ) /* read a line */
                {
                   index++;

                   if(strstr(line,subbuff)!=NULL){
                     printf("%d: ./%s -> %s",index, filename, line);
                 }
                }
             fclose(fptr);

             closedir(d);



    }else if(mode ==2)
    {

      d=opendir(path);
      char path2[80];
      int num=0;



        if (d)
        {
            while ((dir = readdir(d)) != NULL && dir->d_name!="./")
            {
              memset(path2, 0, MAX_LINE * sizeof(char));
              strcat(path2,"./");

             strcat(path2,dir->d_name);
              DIR* dire = opendir(path2);
              if(dire)
              {
                codeSearch(path2,2,keyword );
                closedir(dire);
              }else if (ENOENT == errno){
                return 0;
              }

                fptr = fopen( path2, "r");
               char line [ 500 ]; /* or other suitable maximum line size */
               index=0;
                while ( fgets ( line, sizeof line, fptr ) != NULL ) /* read a line */
               {
                  index++;
                  if(strstr(line,keyword)!=NULL){
                    printf("%d: %s -> %s",index, dir->d_name, line);
                }
               }
               fclose(fptr);
            }


            closedir(d);
        }


    }else
    {
      d = opendir(".");
        if (d)
        {
            while ((dir = readdir(d)) != NULL)
            {
                fptr = fopen( dir->d_name, "r");
               char line [ 500 ]; /* or other suitable maximum line size */
               index=0;
                while ( fgets ( line, sizeof line, fptr ) != NULL ) /* read a line */
               {
                  index++;
                  if(strstr(line,keyword)!=NULL){
                    printf("%d: ./%s -> %s",index, dir->d_name, line);
                }
               }
            }
            fclose(fptr);
            closedir(d);
        }
    }
}
  int concatKeyword(char result[], char *args[])
  {

    memset(result, 0, MAX_LINE * sizeof(char));
    int index=0;
    char current[80];
    int len;
    char subc[80];
    int notYet = 0;
    int return_lenght=0;



    while(args[index]!=NULL)
    {
      printf("got here\n");
      memset(current, 0, MAX_LINE * sizeof(char));
      memset(subc, 0, MAX_LINE * sizeof(char));
      index=index+1;
      strcat(current,args[index]);
      len = strlen(current);

      if(current[0]=='"' && current[len-1] == '"')
      {
        return_lenght=return_lenght+1;

        memcpy( subc, &current[1], len-2);
        strcat(result, subc);
        break;
      }

      else if(current[0]=='"')
      {
        memcpy( subc, &current[1], len-1);
        strcat(result, subc);
        strcat(result, " ");
        return_lenght=return_lenght+1;
        notYet = 1;
      }
      else if(current[len-1] == '"')
      {
        return_lenght=return_lenght+1;
        memcpy( subc, &current[0], len-1);
        strcat(result,subc);

        break;
      }
      else if(notYet!=0)
      {
          return_lenght=return_lenght+1;
          strcat(result,current);
          strcat(result, " ");

      }else  memset(current, 0, MAX_LINE * sizeof(char));
    }
    printf("%s\n",result );
    printf("%d\n",return_lenght );
    return return_lenght;
  }
