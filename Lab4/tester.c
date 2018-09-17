/*
This is tester program to see if my code runs or not.
The following code will create a directory called dir1 which is in the root, then create a directory called dir2 in dir1, and then create a file in dir2. From there I wrote some characters into that file and then called my read to see what is in the file. I then deleted file and unmapped drive
*/

#include "myfileSys.h"
#include <stdio.h>

int main(int argc, char **argv){
	int fd, size;
	char *str, str2[512];
	str = "This is in file";
	size = strlen(str);

	// Mapp drive
	map();

	// initialize root diretory
	initRoot();

	// Create directory
	my_create("dir1", "root", ".txt", 1);

	// Create another directory in directory 1
	my_create("dir2", "dir1", ".txt", 1);

	//Creates a file in directory 2
	my_create("file", "dir2",".txt",0);
	
	//Opens the file using myopen
	fd=my_open("file");
	
	//Write to file using mywrite
	my_write(fd,str,size);

	//Reads from file using myread
	my_read(fd,str2,512);
	//Prints message
	printf("The following was written in file: %s\n", str2);
	
	//Closes the file using my close	
	my_close(&fd);
	
	//Deletes the file using my delete
	my_delete("file");

	// Unmaps 2MB drive
	unmap();
}

