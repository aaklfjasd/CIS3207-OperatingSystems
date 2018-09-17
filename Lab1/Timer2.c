#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

/*Pratik Patel
CIS 3207
Prof: John Fiore 

Timer2.c will create two processes of the application program which itself runs for several minutes. The processes are created by using fork and exec where the start time is recorded before forking and the end time is recorded in the application part of the program which both are written to a text file. The purpose of the program is to measure how long it takes a process to run while another process is concurrently running.
*/

int main()
{
/*Formats the start time and checks if any error occured retreiving value*/
        struct timeval start;
        int rc=gettimeofday(&start,NULL);
        if(rc==0) {
                printf("gettimeofday() in timer successful.\n");
                printf("time = %f\n", (double)start.tv_sec  + (double)start.tv_usec/1000000);
                  }
        else {
        printf("gettimeofday() failed, errno = %d\n",
                errno);
        return -1;
        }

/*The following is the creation of a process using the Application program by forking and executing*/
        pid_t pid1, pid2;
        pid1= fork();
        //Creates a file to put the start times after fork
        if (pid1 < 0)
         {
                 printf("A fork error has occurred.\n");
                 exit(-1);
         }
        else if (pid1 == 0) /* We are in the child. */
         {
                printf("I am the child, about to call execv.\n");
        /*Passing the name of the text file in which to add times
        Passing the name of the file the process(Application) will use */
                char *argv[] ={"application.out","garbage.txt","myfile.txt",NULL};
                execv("application.out",argv);
        /*  If execlv() is successful, we should not reach this next line. */
                printf("The call to execv() was not successful.\n");
                exit(127);
         }
      else {  
 	/*We are going to fork again now to create another process*/
	 struct timeval start2;
         rc=gettimeofday(&start2,NULL);
         if(rc==0) {
                printf("\ngettimeofday2() in timer successful.\n");
                printf("time2 = %f\n", (double)start2.tv_sec  + (double)start2.tv_usec/1000000);
                  }
         else {
         printf("gettimeofday2() failed, errno = %d\n", errno);
         return -1;
         }
	 pid2=fork();
         if(pid2<0){
                printf("Fork Error");}
         else if(pid2==0){
                 printf("I am the child2, about to call execv again.\n");
                 /*Passing the name of the text file in which to add times
                 Passing the name of the file the process(Application) will use */
                char *argv[] ={"application.out","timer2.txt","myfile2.txt",NULL};
                execv("application.out",argv);
        /*  If execlv() is successful, we should not reach this next line. */
                printf("The call to execv() was not successful.\n");
                exit(127);
                }
         else  /* We are in the parent. */
         {
         int status;
	 waitpid(pid1,&status,0);
	 waitpid(pid2,&status,0);               /* Wait for the child to terminate. */
        //Creates a file to put the start times after fork
	 FILE * fptime2;
         fptime2= fopen("timer2.txt","a+");
         fprintf(fptime2,"%f\n",(double)start2.tv_sec  + (double)start2.tv_usec/1000000);
         //get time interval
	fseek(fptime2,-26,SEEK_END);
         char endtimer[7];
         fread(endtimer,sizeof(char),7,fptime2);
         double holder=atof(endtimer);
         printf("\nTime Interval for 2nd process was %f seconds",holder-( (double)start.tv_usec/1000000));
	 fclose(fptime2);
	 printf("\nI am the parent.  The child just ended.  I will now exit.\n");
         //exit(0);
         }
	}
return 0;
}

	
