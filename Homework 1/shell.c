/*
  Implements a minimal shell.  The shell simply finds executables by
  searching the directories in the PATH environment variable.
  Specified executable are run in a child process.

  AUTHOR: Derek Low
*/

#include "shell.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <ctype.h>

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
  pathEnv = (char *) getenv("PATH");
  //printf("pathEnv is length %lu\n",strlen(pathEnv));

  if (pathEnv == NULL) return 0; /* No path var. That's ok.*/
  /*Allocation of memory to thePath must be as large as the pathEnv to copy
    successfully*/
  thePath = (char *) malloc(sizeof(char)*strlen(pathEnv)+1);
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
  i = 0;
  /* Realized that I was missing the first entry after I completed the while
     loop, so I'm doing that here.
  */
  dirs[i] = nextcharptr;
  while (*nextcharptr){
    /*
      Replace all colons with then null terminator, then iterate nextcharptr and i.
      After i iteration, set the next dirs pointer to the iterated nextcharptr.
     */
    if (*nextcharptr == ':'){
        while(*nextcharptr==':'){//handle that double colon case
            *nextcharptr = '\0';
        }
      nextcharptr++;
      //iterate to the next item so that it is possible to set the next dirs entry to the character
      //after a colon in the path
      i++;
      dirs[i] = nextcharptr;
    }
    else nextcharptr++;
  }
  numDirs = i+1 ;


  /* Print all dirs */
#ifdef DEBUG
  for (i = 0; i < numDirs; i++) {
    printf("%s\n",dirs[i]);
  }
#endif
  //prevent a memory leak.
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

  // cmd->argv[argc] = (char *) malloc(MAX_ARG_LEN);

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
//I'm preallocating because I run into segfaults if I don't, especially in the printJobs function.
void preallocating(struct Jobs *job[]){
    int numJobs;
    for (numJobs=0;numJobs<MAX_JOBS;numJobs++){
        //allocate memory, fill the values with random stuff
        job[numJobs] = (struct Jobs *) malloc(sizeof(struct Jobs));
        job[numJobs]->id = 0;
        job[numJobs]->pid = 0;
        job[numJobs]->exename="Blank";
    }
}
//print jobs will iterate through the list and print out all processes that are do not have a pid of 0
// the only processes that have a pid of 0 are the preallocated ones
void printJobs(struct Jobs *list[], int size){
    int i;
    printf("\tJob ID\tProcess ID\tProcess Name\n");
    //printf("%s\n",list[0]->exename);
    for (i = 0; i < size; i ++){
        if (list[i]->pid != 0){
            printf("%d\t%d\t%d\t\t%s\n",i+1,list[i]->id,list[i]->pid, list[i]->exename);
        }
    }
}

//Adding jobs will add a job at the next free index
void addJobs(struct Jobs *job[], int pid, char *exe, int counter){
    int i = 0;
    while (job[i]->pid != 0) i++; //check for a pid of 0 to find a free job index
    free(job[i]); //free the job that is not a real job
    job[i] = (struct Jobs *) malloc(sizeof(struct Jobs)); //allocate mem for the new job
    //fill the appropriate fields
    job[i]->id = counter;
    job[i]->pid = pid;
    job[i]->exename = (char *) malloc(strlen(exe)+1);//malloc space for a string copy of the cmd name
    strcpy(job[i]->exename,exe);
}
//killing jobs
void killJob(struct Jobs *job[],int killTarget){
    int i;
    int found = 0;//"boolean", not really necessary
    for (i=0; i <MAX_JOBS;i++){
        if (job[i]->id == killTarget){
            //first, kill the process
            kill(job[i]->pid,SIGKILL);
            free(job[i]->exename);//free the cmd name
            free(job[i]);//free the job struct memory
            found = 1;
            printf("Your requested process has been killed.\n");//inform that the process has been successfully killed.
            job[i] = (struct Jobs *) malloc(sizeof(struct Jobs));//fill the remaining with a random job
            job[i]->pid = 0;//set pid for purposes of print and add
            break;//leave the function
        }
    }
    if (found == 0) printf("Could not locate process.\n");// could not locate the pid
}

/*
  Runs simple shell.
*/
int main(int argc, char *argv[]) {

  char *dirs[MAX_PATHS]; // list of dirs in environment
  int numPaths;
  char cmdline[LINE_LEN];
  int argcount = 0;
  struct Command *cmd = (struct Command *) malloc(sizeof(struct Command));
  struct Jobs *job[MAX_JOBS];

  pid_t pid;
  int numJobs,i,status;
  char buffer[512];
  int jobCount = 1;//so that every process has a unique ID

  preallocating(job);
  numPaths = parsePath(dirs);

  /*This is where the infinite loop starts for the shell. Could have done while(true), but the for loop seems to be
    the common version.
  */ 

  for(;;){
    //request user command
    getcwd(buffer,512);
    printf("Please provide an input.You are currently in:\n%s\n",buffer);//print the current directory
    fflush(stdout);    
    //we check the entered command because we do not want null or new line commands, since those are not instructions
    //for the shell to do anything. Instead, the shell will reprompt, since it is in a conditional.
    /*for (numJobs = 0; numJobs < MAX_JOBS; numJobs ++){
        if (job[numJobs]->exename != "Blank"){
            printf("%d\t%d\t%d\t%s\n",numJobs,job[numJobs]->id,job[numJobs]->pid,job[numJobs]->exename);
        }
    }*/
    //check to make sure there are no processes in the background jobs list that have already finished running.
    for (i=0;i<MAX_JOBS;i++){
        if(waitpid(job[i]->pid,&status,WNOHANG)!=0){
            free(job[i]);
            job[i] = (struct Jobs *) malloc(sizeof(struct Jobs));//fill the freed position to prevent a seg fault when checking
        }
    }

    if (fgets(cmdline,LINE_LEN,stdin) != NULL && cmdline[0] != '\n'){//request a command 
        //call parseCmd function to parse the different portions of the command entered, since there may be 
        //multiple arguments in a command
      	argcount = parseCmd(cmdline,cmd);
        //place the built in commands first, since they are not supposed to spawn a child process when called.
        //exit the shell
        if ((strcmp(cmd->argv[0],"exit")) == 0){
            printf("Exiting.\n");
            break;
        }//first check to make sure there are no jobs in the list that are done running, then print the list of current bg processes
        else if ((strcmp(cmd->argv[0],"jobs")) == 0){
            for (i=0;i<MAX_JOBS;i++){
                if(waitpid(job[i]->pid,&status,WNOHANG)!=0){//check if the child process of a particular pid is complete or not
                    free(job[i]);
                    job[i] = (struct Jobs *) malloc(sizeof(struct Jobs));
                }
            }
            printJobs(job, MAX_JOBS);
        }//look for a process to kill.
        else if ((strcmp(cmd->argv[0],"kill")) == 0 ){
            if (cmd->argv[1] == NULL) printf("Second argument must be a job ID.\n");//in the case of no second argument
            else {
                int requestedID = atoi(cmd->argv[1]);//ascii 2nd argument to int
                if (requestedID == 0) printf("That is not a valid job ID.\n");//atoi returns 0 if the ascii isnt an int
                else {
                    printf("The requested PID is %d\n",requestedID);
                    killJob(job,requestedID);//run the killJob with the params list of jobs and the requested ID of the job.
                }
            }
        }
    	else if ((pid=fork())!= 0){//this is the parent process
            if ((strcmp(cmd->argv[argcount-1],"&"))!=0){
                //if there is no ampersand at the end of the input, we want to wait for the children to finish
                //before we allow the parent process to continue.
                int status;
                waitpid(pid,&status,0);
            }
            else if ((strcmp(cmd->argv[argcount-1],"&"))==0){
                int arraySize = 1;
                for (i = 0;i<MAX_JOBS;i++){
                    if (job[i]->pid !=0) arraySize++;
                }
                printf("There are currently %d jobs out of %d possible jobs.\n",arraySize,MAX_JOBS);
                if (arraySize == MAX_JOBS+1){
                    printf("You are requesting more than the permitted amount of background processes. This process will run in foreground.\n");
                    fflush(stdout);//make sure the user sees the message.
                    waitpid(pid,&status,0);
                }
                else{addJobs(job,pid,cmd->argv[0],jobCount);
                jobCount++;
                }
            }
    	}
    	else{//this is the child funtion
            //check for an ampersand, and remove it. the ampersand provides no use to the child process,
            if ((strcmp(cmd->argv[argcount-1],"&"))==0){
                free(cmd->argv[argcount-1]);//free the pointer to the ampersand
                cmd->argv[argcount-1] = NULL;//set the pointer to null
            }
            char *prompt = lookupPath(cmd->argv[0], dirs, numPaths);
            //indicate that there is a process handling the command entered
            //look for the command in the path to see if its a built in function
            //if the prompt cannot be found in the path, then break the loop and ask the user for another prompt
    		if (prompt ==NULL){
    		    break;
    		}
            //if the prompt is found, perform it.
    		execv(prompt,cmd->argv);
    	}

    for (i=0;i<cmd->argc;i++) free(cmd->argv[i]); //free all pointers to all command arguments
    }
  } 
  for (i=0;i<cmd->argc;i++) free(cmd->argv[i]); //free the five byte exit
  //for (i=0;i<MAX_JOBS;i++) free(job->exename);
  for (i=0;i<MAX_JOBS;i++) free(job[i]);//free the job structure
  free(dirs[0]);
  free(cmd);// free the cmd structure
}
