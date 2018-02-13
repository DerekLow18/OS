/*
  Implements a minimal shell.  The shell simply finds executables by
  searching the directories in the PATH environment variable.
  Specified executable are run in a child process.

  AUTHOR: Darren Tirto
*/

#include "shell.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include <sys/wait.h>//this is for waiting for pids

int parsePath(char *dirs[]);
char *lookupPath(char *fname, char **dir,int num);
int parseCmd(char *cmdLine, struct Command *cmd);

/*
  Read PATH environment var and create an array of dirs specified by PATH.

  PRE: dirs allocated to hold MAX_ARGS pointers.
  POST: dirs contains null-terminated list of directories.
  RETURN: number of directories.

  NOTE: Caller must free dirs[0] when no longer needed.

*/
int parsePath(char *dirs[]) {
  int i, numDirs;
  char *pathEnv;
  char *thePath;
  char *nextcharptr; /* point to next char in thePath */

  for (i = 0; i < MAX_PATHS; i++) dirs[i] = NULL;
  pathEnv = (char *) getenv("PATH"); //store the C string containing PATH

  thePath = (char *) malloc(sizeof(char)*(strlen(pathEnv)+1));//Allocate same memory amount for thePath as pathEnv

  if (pathEnv == NULL) return 0; /* No path var. That's ok.*/

  /* for safety copy from pathEnv into thePath */
  strcpy(thePath,pathEnv);

#ifdef DEBUG
  printf("Path: %s\n",thePath);
#endif

  /* Now parse thePath */
  nextcharptr = thePath;
  /* Eliminate extra delimiters. */
  while (*nextcharptr == DELIM) nextcharptr++;
  if (*nextcharptr == 0) {
    fprintf(stderr,"No paths present.\n");
    return 0;
  }

  /* 
     Find all substrings delimited by DELIM.  Make a dir element
     point to each substring.
     TODO: approx a dozen lines.
  */
  dirs[0]=nextcharptr;
  numDirs=1;
  while(*nextcharptr!= '\0'){//Loop through entire string
    if(*nextcharptr==DELIM){//if we find delimiter
      while(*nextcharptr==DELIM){//while we run into consecutive delimiters
        *nextcharptr = '\0';//change delimiter to string terminator
        *nextcharptr++;//keep going
      }
      dirs[numDirs]=nextcharptr;//work on next dir index
      numDirs++;//
    }
    if(numDirs==MAX_PATHS){//If we hit the MAX_PATH LIMIT
      printf("Max path of %d reached. Rest of path placed in dirs[%d] \n",numDirs,numDirs );
      break;
    }
    *nextcharptr++;
  }

  /*testing*/

  



  /* Print all dirs */
#ifdef DEBUG
  for (i = 0; i < numDirs; i++) {
    printf("%s\n",dirs[i]);
  }
#endif
    
  return numDirs;
}


/*
  Search directories in dir to see if fname appears there.  This
  procedure is correct!

  PRE dir is valid array of directories
  PARAMS
   fname: file name
   dir: array of directories
   num: number of directories.  Must be >= 0.

  RETURNS full path to file name if found.  Otherwise, return NULL.

  NOTE: Caller must free returned pointer.
*/

char *lookupPath(char *fname, char **dir,int num) {
  char *fullName; // resultant name
  int maxlen; // max length copied or concatenated.
  int i;

  fullName = (char *) malloc(MAX_PATH_LEN);
  /* Check whether filename is an absolute path.*/
  if (fname[0] == '/') {
    strncpy(fullName,fname,MAX_PATH_LEN-1);
    if (access(fullName, F_OK) == 0) {
      return fullName;
    }
  }

  /* Look in directories of PATH.  Use access() to find file there. */
  else {
    for (i = 0; i < num; i++) {
      // create fullName
      maxlen = MAX_PATH_LEN - 1;
      strncpy(fullName,dir[i],maxlen);
      maxlen -= strlen(dir[i]);
      strncat(fullName,"/",maxlen);
      maxlen -= 1;
      strncat(fullName,fname,maxlen);
      // OK, file found; return its full name.
      if (access(fullName, F_OK) == 0) {
	return fullName;
      }
    }
  }
  fprintf(stderr,"%s: command not found\n",fname);
  free(fullName);
  return NULL;
}

/*
  Parse command line and fill the cmd structure.

  PRE 
   cmdLine contains valid string to parse.
   cmd points to valid struct.
  PST 
   cmd filled, null terminated.
  RETURNS arg count

  Note: caller must free cmd->argv[0..argc]

*/
int parseCmd(char *cmdLine, struct Command *cmd) {
  char *clptr = cmdLine;
  int maxlen = strlen(cmdLine);
  int argc = 0; // arg count
  int len; // length of arg

  //cmd->argv[argc] = (char *) malloc(MAX_ARG_LEN); //double allocation

  // extract args up to max number
  while (argc < MAX_ARGS-1) {
    // find next arg
    while (*clptr != 0 && isspace(*clptr)) clptr++;
    if (*clptr == 0) break; // no next arg, just space
    // go to next whitespace, keep length
    len = 0;  
    while (*clptr != 0 && !isspace(*clptr)) {
      clptr++;
      len++;
    }
    // copy over substring
    cmd->argv[argc] = (char *) malloc(len+1);
    strncpy(cmd->argv[argc],(clptr-len),len);
    cmd->argv[argc][len] = 0;
    /*    printf("ZZ %s\n",cmd->argv[argc]); /* Debug */
    argc++;
  }
  cmd->argv[argc] = NULL;
  cmd->argc = argc;
  return argc;
}

/*
  Runs simple shell.
*/
int main(int argc, char *argv[]) {

  char *dirs[MAX_PATHS]; // list of dirs in environment
  int numPaths;
  char cmdline[LINE_LEN];
  int pid;
  char buffer[512];

  //was initialized but no memory allocated
  struct Command *cmd = (struct Command *) malloc(sizeof(struct Command));

  numPaths = parsePath(dirs);
  

  // TODO
  while (TRUE) {
    //printf("%s: ",getPrompt(prompt,LINE_LEN));  // next prompt!!
    getcwd(buffer,512);
    printf("%s$ ", buffer);
    fflush(stdout);
    if (fgets(cmdline,LINE_LEN,stdin) != NULL && cmdline[0] != '\n') {
      parseCmd(cmdline,cmd);
      for (int i = 0; i < cmd->argc; i++) {
        printf("arg[%d]: %s\n",i,cmd->argv[i]);
      }
      if ( strcmp(cmd->argv[0],"exit") == 0) {
        printf("Exiting.\n");
        break;
      }

      else{
        if((pid=fork())!=0){//this means we are the parent 
          //printf("Parent Fork\n");
          if(*(cmd->argv[cmd->argc-1])!='&'){//if there's no & then wait for child to finish.
            int status;
            waitpid(pid,&status,0);
           // printf("done waiting for %s with status %d\n", (cmd->argv[cmd->argc-1]), status);
          }
        }else{//we are a child
          if(*(cmd->argv[cmd->argc-1])=='&'){
            free(cmd->argv[cmd->argc-1]);//strip out the & so we can run the command
            cmd->argv[cmd->argc-1]=NULL;//turn the free space to NULL
          }
          char *p = lookupPath(cmd->argv[0],dirs,numPaths);//find the path of the command
		  if (p==NULL){//If command not found
            break;//kill the (child) process
          }
          execv(p,cmd->argv);//execute the command
          free(p);//free up the path of the command
        }

      }

      for (int i=0; i<cmd->argc; i++) free(cmd->argv[i]);
    }
  }
  for (int i=0; i<cmd->argc; i++) free(cmd->argv[i]);//free up the exit command
  free(cmd);
  free(dirs[0]);
}

