

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>


/*Changes the current directory*/
int shell_cd(char *args[])
{
/*If the arguments is null print current working directory*/
 if(args[1]==NULL)
	{
	char buf[50];
	getcwd(buf,sizeof(buf));
	printf("Current Directory is: %s\n",buf);   
	return 1;
	}
 else
	/*Attempt to change directory to agruments given if does not work
	  print error message*/
	{
	 if(chdir(args[1])!=0)
 	 {printf("Error in cd directory does not exist \n");
	 return -1;
	 }
	/*If chdir works update path variable*/
	else
	 {
	 if(setenv("PWD",args[1],1)!=0)
		printf("Error in updating path\n");
 	 }
	}
return 1;
}

/*Clears the screen*/
int shell_clr()
{
 int i;
for(i=0;i<100;i++)
	printf("\n");
return 1;
}

/*Prints the elements in the directory given*/
int shell_dir(char *args[])
{
	DIR *dp; struct dirent *ep;
	/*If agruments are NULL print current directory elements*/
	if(args[1]==NULL){
		if((dp=opendir("./"))==NULL)
		       {printf("Error in openingdir\n");}
		else {
		 while((ep=readdir(dp))!=NULL)
       		  {
                  printf("%s\n",ep->d_name);
       		  } 
		closedir(dp);
		}
	}
	/*Attempt to open direcotry name given and display contents*/
	else{
		if((dp=opendir(args[1]))==NULL)
			{printf("Error in opening dir\n");}
		else
		{
		 while((ep=readdir(dp))!=NULL)
       		 {printf("%s\n",ep->d_name); }
		closedir(dp);
		}
	}
return 1;
}

/* Prints all environment variables*/
int shell_environ()
{
 extern char **environ;
 int i=0;
 while(environ[i]!=NULL)
	{
	printf("%s\n",environ[i]);
	i++;
	}
 return 1;
}

/*Displays the arguments written after echo command as strings*/
int shell_echo(char *args[])
{
	if(args[1]==NULL)
	{ printf("No arguments written\n");return -1;}
	else
	{
	 /*Creates space for a line 300 characters long*/
 	 char *string=malloc(sizeof(char*)*300);
  	int i=1;
	/*Joins all separte string arguments together with only one space  bewteen all strings*/	
  	while(args[i]!=NULL)
	{
	 strcat(string,args[i]);
	 strcat(string, " ");
	i++;
	}
	printf("%s\n",string);
	free(string);
	return 1;
	}
}

/*Opens the readme textfile  */
int shell_help()
{
	FILE *fp; char buf[200];
	fp=fopen("readme","r");
	if(fp==NULL)
	{printf("Error in opening file\n"); return -1;}
	else
	{
	 while(fgets(buf,sizeof(buf),fp)!=NULL)
		{
		printf("%s",buf);
		}

	printf("\n");
	return 1;
	}
}

/*Pauses the shell by entering a infinite loop until user enters newline character*/
int shell_pause()
{
 char hold;
 printf("Shell is paused. Press enter to continue\n");
 while((hold=getchar())!='\n')
	{/*Do Nothing*/}

 return 1;
}

/*Quits entire myshell program*/
int shell_quit()
{
 exit(0);return 1;
}


