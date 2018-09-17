#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "myshell.h"


/*List of the functions and list of pointer to functions*/
char *builtinCommands[]= {"cd","clr","dir","environ","echo","help","pause","quit"};

int (*builtinPointer[]) (char **) ={&shell_cd,&shell_clr,&shell_dir,&shell_environ,&shell_echo,&shell_help,&shell_pause,&shell_quit};


int main(int argc, char**argv)
{
 shellmain(argv);
 return 1;
}


/*Will check if batchfile exists if not then constantly print shell prompt*/
void shellmain(char **argv)
{
 FILE *fp; char**arg; char *line=(char *) malloc(300);
 printf("Enter 'quit' to stop shell\n");
//IF argv is Not NULL Means that a batchfile exists
 if(argv[1] != NULL)
 { 
 	if(strcmp(argv[1],"batchfile")==0)
	{
	 if((fp=fopen(argv[1],"r"))==NULL)
	 	{printf("Error opening batchfile\n");}
	/*Opening batchfile is successful and now reading from batch file*/
	 while(fgets(line,300,fp))
		{
		/*Checks if quit was entered as no point in continuing*/
		 if(strcmp(line,"quit")==0)
		  {shell_quit();}
		 /*Parses line and then execs line*/
		 arg=parser(line);
		 myexec(arg);
		}
	fclose(fp);
	shell_quit();
	}
 }
/*No Batchfile exists so just run shell prompt forever until quit*/
else
{
 while(1)
 {
  line=Prompt_Read();
  arg=parser(line);
  if(strcmp(line,"quit")==0)
   {shell_quit();}
  myexec(arg);
 }
}
//shell_quit() DONT REALLY NEED
}


/*Prompts the command line and reads user input*/
char *Prompt_Read()
{
 char *ln, c;
 char currdir[50];
 if((ln=(char*)malloc(300))==NULL)
	{ printf("Error with malloc\n");exit(-1);}
 if ( getcwd(currdir, sizeof(currdir)) == NULL)
	{printf("Error getting directory\n");exit(-1);}	
 
 printf("%s>",currdir);
 fgets(ln,300,stdin);
 return ln;
}


/* Parses the arguments passed by looking for spaces and separtating them*/
 char **parser(char* str)
{
 char **args= malloc(40);
 char delimit[]=" \t\r\n\a\f";
 char *hold; int i=0;
 hold= strtok(str,delimit);
 

 while(hold!=NULL)
 { args[i]=malloc(strlen(hold+1));
  args[i]=hold;
  hold=strtok(NULL,delimit);
  i++;
 }
args[i]=NULL;
return args;
}

/* Searches for symbols in string array and stores index if found*/
int *flagger(char **args)
{
    int i=0;
    int lsym=-1; //flag spot for <
    int gsym=-1; //flag spot for >
    int g2sym=-1; //flag spot for >>
    int pipe=-1; //flag spot for pipe |
    int *Returned_array = malloc(4 *sizeof(int));

    /* Searches array string input for symbols if appropriate symbol is found,
     or -1 if not found */
    while (args[i] != NULL)
    {
        if (args[i][0] == '<')
        {
            lsym = i;
        }
        else if (args[i][0] == '>')
        {
            if (args[i][1] == '>')
            {
                g2sym = i;
            }
            else
            {
                gsym = i;
            }
        }
        else if(args[i][0] == '|')
        {
            pipe=i;
        }
        i++;
    }

    // Stores indexes of each symbol in array
    Returned_array[0] = lsym;
    Returned_array[1] = gsym;
    Returned_array[2] = g2sym;
    Returned_array[3] = pipe;

    return Returned_array;
}


/*Will call the appropriate call to a function depending on if pipe, redirect or just buildin*/
int myexec(char **args)
{
    int i;
    int *flag = flagger(args);

    // Leaves function if inputs are empty
    if(args[0] == NULL)
    {
        return 1;
    }

    // When symbols are found it goes to the appropriate launch features
    for(i=0; i<4; i++)
    {
        if(flag[i] >= 0)
        {
            if(i!=3)
		{return progredirect(args, flag);}
	    else
		{return prog(args,flag);}
        }
    }

    // Runs the built-in if only arguments are builtin with no redirect or pipe
    for(i=0; i<8; i++)
    {
        if( strcmp(args[0], builtinCommands[i])==0 )
        {
            return builtinPointer[i](args);
        }
    }

    // runs launcher
    return prog(args, flag);
}

/* Changes file descriptors and exec depending on redirection*/
int progredirect(char **args, int flag[])
{
 pid_t pid;
 int status, backg=0, i=0, j=0;
 char *args1[10];
 int saved_stdout=dup(1);
 int saved_stdin=dup(0);
  
// Looks for '&' marker
   i=0;
   while((args[i] != NULL) && (backg==0))
    {
        if (strcmp(args[i],"&")==0)
        {
            backg=1;
        }
        i++;
    } 
int file;
    // looks for '<'
    if(flag[0] >= 0)
    {
        file = open(args[flag[0]+1 ], O_RDONLY);
        dup2(file,0);
   	for(i=0; i<flag[0]; i++)
        {
            args1[i] = args[i];
        }
    	args1[i]=NULL;
    }
    // looks for '>'
    if(flag[1] >= 0)
    {
        file = open(args[flag[1]+1 ], O_WRONLY|O_CREAT|O_TRUNC,S_IRWXU|S_IRWXG|S_IRWXO);
        dup2(file,1);
	for(i=0; i<flag[1]; i++)
        {
            args1[i] = args[i];
        }
	args1[i]=NULL;
	
    }
   // looks for '>'
    else if(flag[2] >= 0)
    {
        file = open(args[ flag[2]+1 ], O_WRONLY|O_CREAT|O_APPEND,S_IRWXU|S_IRWXG|S_IRWXO);
        dup2(file,1);
	for(i=0; i<flag[2]; i++)
        {
            args1[i] = args[i];
        }
	args1[i]=NULL;
    }
     	
     while(i<8)
        {
            if( strcmp(args[0], builtinCommands[i])==0 )
            {
                builtinPointer[i](args1);
                i=10;j=1;
            }
        i++;
        }
     //If a buildin was not used fork and exec prog	
     if(j!=1)
	{
	   // Forks
   	 if ((pid=fork()) < 0)
   	 {
        	perror("FORK");
         }
    	// Child which execs argument
    	else if (pid==0)
    	{
	 if (execv(args1[0], args1)<0)
                 {
                perror("\nLaunch 1\n");
                 }
                 // Exits if executes fails
                exit(EXIT_FAILURE);
	}
	//Parent
	else
	{
	//Wait if only no background operator found
	if(backg==0)
	{wait(&status);} 	 	
	}
       }
//Reset the in and out to originial file descriptors
  dup2(saved_stdin,0);
  dup2(saved_stdout,1);
  close(saved_stdout);
  close(saved_stdin);
  close(file);
  return 1;
}

/*Will fork and exec and change file descriptors due to pipeing*/
int prog(char **args, int flag[])
{
//Store initial file descriptors
 int saved_stdout=dup(1);
 int saved_stdin=dup(0);
    pid_t pid, pid2;
    int status, backg=0, i=0, j=0;
    int fd[2];

    // Initialize sized arguments for pipe
    char *arg1[10];
    char *arg2[10];

    // Looks for '&' marker
    i=0;
    while((args[i] != NULL)&&(backg==0))
    {
        if (strcmp(args[i],"&")==0)
        {
            backg=i;
        }
        i++;
    }

    // If pipe exists, separates the 2 program's arguments
    if (flag[3] >= 0)
    {
        pipe(fd);

        // Stores first arguments
        for(i=0; i<flag[3]; i++)
        {
            arg1[i] = args[i];
        }
	arg1[i]=NULL;
	i=i+1;
        // Stores second arguments
        for(j=0; args[i]!=NULL; i++)
        {
	 if(i!=backg)
	  {
            arg2[j] = args[i];
       	    j++;
	  }
        }
	arg2[j]=NULL;
    }

    // Forks
    if ((pid=fork()) < 0)
    {
        perror("FORK");
    }
    // Child
    else if (pid==0)
    {

        // Looks for redirection or pipe
        if(flag[3] >= 0)
        {
            // Changes file descriptor for pipes
            close(fd[0]);
            dup2(fd[1], 1);
        }
	i=0;j=0;
        // Runs built-in
        while(i<8)
        {
            if( strcmp(args[0], builtinCommands[i])==0 )
            {
                builtinPointer[i](args);
                i=10;j=1;
            }
	i++;
        }
	if(j!=1)
	{
       	 // Executes function if it is not built-in
       	 if (execv(arg1[0], arg1) == -1)
       		 {
            	perror("\nLaunch 1\n");
       		 }
       		 // Exits if executes fails
        	exit(EXIT_FAILURE);
	}
        // Parent Process
    }
    else
    {
        // Forks again for pipe
        if(flag[3] >= 0)
        {
            if ((pid2=fork()) < 0)
            {
                perror("FORK");
            }
            // 2nd Child
            else if (pid2==0)
            {
                // Changes file descriptor
                close(fd[1]);
                dup2(fd[0], 0);
		
		i=0;j=0;
                // Runs built-in
                while( i<8)
                {
                    if(strcmp(arg2[0], builtinCommands[i])==0 )
                    {
                        builtinPointer[i](arg2);
                        i=10;j=1;
                    }
		i++;
                }
		if(j!=1)
		{			
                // Executes function if it is not built-in
               		 if (execv(arg2[0], arg2) == -1)
               		 {
                    	perror("\nLaunch 2\n");
               		 }
               		 exit(EXIT_FAILURE);
           	}
	    }
            else
            {
                // Wait if no background operator found
		if(backg==0)
                {
                    waitpid(pid,&status,0);
                    waitpid(pid2,&status,0);
                }
		//Resets file descriptors to original
		dup2(saved_stdin,0);
 		dup2(saved_stdout,1);
 	        close(saved_stdout);
  		close(saved_stdin);
		return 1;
            }
        }

    }
return 1;
}

