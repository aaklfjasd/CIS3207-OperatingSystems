#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <errno.h>
#include <math.h>


/*Pratik Patel
CIS 3207
Prof: John Fiore 

Application.c will create a file with 10 unique random records, which then one of the records will be randomly selected and compared with the record saved in memory. The entire procedure will iterated so that the program will run for several minutes. The purpose of this program is to create a program which will run for a long time which will be later used by Timer1 and Timer2. 
*/


int main(int argc,char*argv[])
{
/*Retreives the current time of system and checks if any errors retreving time occurs*/
struct timeval end;
int rc=gettimeofday(&end,NULL);
if(rc==0) {
        printf("gettimeofday() in application successful.\n");
        printf("time = %f\n", (double)end.tv_sec  + (double)end.tv_usec/1000000);
    }
    else {
        printf("gettimeofday() failed, errno = %d\n",
                errno);
        return -1;
    }
/*Uses the parameters passed by the Timers program to write the end time in appropriate timer textfile*/
FILE * fptime1;
fptime1= fopen(argv[1],"a+");    
fprintf(fptime1,"%f,",(double)end.tv_sec  + (double)end.tv_usec/1000000);
fclose(fptime1);


/*The following is the application that will write,read,and compare records continusly for a few minutes*/
int timerloop=1;
srand(time(NULL));
//CHANGED VALUE BACK TO 15000 MADE SLOWER TO TEST
while(timerloop<30000)
  {	
	double runlonger=sqrt(M_PI)*1.32314352352325+1.23432425231*12132.0;
	runlonger=exp(runlonger);
	FILE *fp;
	char buffer[10][120];
	int i=0;int k=0;
	fp=fopen(argv[2],"w+");
	fseek(fp,0,SEEK_SET);
	for(k=0;k<10;k++)
        {
		for(i=0;i<120;i++)
		{  
		//random generate a number from 33 to 125 due to ASCII
		buffer[k][i]=(char) (rand()%92)+33;	
		}
	fwrite(buffer[k],sizeof(char),sizeof(buffer[k]),fp);
	//To create a new line in text file
	fwrite("\n",sizeof(char),1,fp);
        }
	//Generates a number bewteen 0-10 so find a record to compare to 
	int rand1=rand()%10;
	int offset=0;
	//to get the offset amount in file 120 characters plus the new line
	if (rand1>0)
		offset=rand1*121;
	//Set postion in file to record line and read the record into compare array
	fseek(fp,offset,SEEK_SET);
	char compare[120];
	int read=fread(compare,sizeof(char),120,fp);
	if(read!=120)
	{
		printf("\nError in reading record");
		exit(1);
	}
	//Prints the letters that were read in file
	/* Commented out as was used to test that records were unique and randomly compared each iteration
	printf("\n%d\n",rand1);
	for(i=0;i<120;i++)
	{
		printf("%c",compare[i]);
      	}
	printf("\n");
	*/
	i=0;
	//Checks if record in memory is the same as record that was just read
	while(i<120)
	{	if(compare[i]!= buffer[rand1][i])
			{ printf("\nError the records are not the same");
			exit(1);
			}
		i++;	
	}
	//printf("Record was compared succesfully\n");
	fclose(fp);
	timerloop++;
   }
int rem = remove(argv[2]);

   if(rem == 0) 
   {
      printf("File deleted successfully %s \n",argv[2]);
   }
   else 
   {
      printf("Error: unable to delete the file\n");
   }
     
return 0;
}


