#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

/*Pratik Patel
CIS 3207
Prof: John Fiore 

Timer1.c will create a process of the application program which itself runs for several minutes. The process is created by using fork and exec where the start time is recorded before forking and the end time is recorded in the application part of the program which both are written to a text file. The purpose of the program is to measure how long it takes a process to load after it has been created.
*/


int main()
{
/*Formats the start time and checks if any error occured retreiving value*/
	struct timeval start;
	int rc=gettimeofday(&start,NULL);
	if(rc!=0) {
        printf("gettimeofday() failed, errno = %d\n",
                errno);
         return -1;
    	}
	
/*The following is the creation of a process using the Application program by forking and executing*/
	pid_t pid;
	pid= fork();
	//Creates a file to put the start times after fork

	if (pid < 0)
 	 {
  		 printf("A fork error has occurred.\n");
  		 exit(-1);
 	 }
 	else if (pid == 0) /* We are in the child. */
  	 {
    		printf("I am the child, about to call execv.\n");
	/*Passing the name of the text file in which to add times
	Passing the name of the file the process(Application) will use */
   		char *argv[] ={"application.out","timer1.txt","myfile.txt",NULL};	
	        execv("application.out",argv);
   	/*  If execlv() is successful, we should not reach this next line. */
    		printf("The call to execv() was not successful.\n");
    		exit(127);
  	 }
 	 else  /* We are in the parent. */
  	 {
	wait(0);               /* Wait for the child to terminate. */
	//Creates a file to put the start times after fork
        FILE * fptime1;
        fptime1= fopen("timer1.txt","a+");
        fprintf(fptime1,"%f\n",(double)start.tv_sec  + (double)start.tv_usec/1000000);
       //To get the time interval
	 fseek(fptime1,-26,SEEK_END);
	char endtime[7];
	fread(endtime,sizeof(char),7,fptime1);
	double holder=atof(endtime);
	printf("\nTime Interval was %f seconds",holder-( (double)start.tv_usec/1000000));
	
	fclose(fptime1);
    	 printf("\nI am the parent.  The child just ended.  I will now exit.\n");
   	 exit(0);
  	 }
return 0;
}

	



