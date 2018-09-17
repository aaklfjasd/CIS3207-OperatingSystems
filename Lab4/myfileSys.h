#ifndef HEADER_FILE
#define HEADER_FILE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>

/*
3896 groups * 512 byters
487 pages * 4096 page size
*/
#define DRIVE_SIZE 2000000
#define FAT_SIZE 3896
#define DATA_SIZE 1994752
#define ROOT_SIZE 1024
#define DIR_ENTRY_SIZE 32
#define GROUP_LENGTH 512
#define NAME_LENGTH 32
#define EXT_LENGTH 8
#define ROOT_INDEX 0
#define DATA_START 2
#define FAT_EOF -2

// Diretory
struct dir
{
	char *name;
	int file[GROUP_LENGTH/DIR_ENTRY_SIZE];
};

/* File entry in directory 
I have put my metadata in a structure of directory entry which contains name, extension, index in my FAT, if the file is a directory or not, and the time it was created.
*/
struct dirEntry
{
	char *name;		
	char *ext;		
	int fatIndex;	// index in fat, -1 if empty
	int isDir;		
	time_t time;	
};

char *fatMap, *dataMap;
int drivePointer;

void map();
void unmap();
void initDir();
void initRoot();
int findFreeFat();
int findFreeDir(int group);
int searchDir(char *dirName, int start);
int searchFile(char *fileName, int start);
int searchFileEntry(int fatIndex, int start);

/*
 Main Functions that will create, delete, open, close, read, and write the files.
*/
void my_create(char *fileName, char *dir, char *ext, int isDir);
int my_delete(char *fileName);
int my_open(char *fileName);
void my_close(int *fd);
void my_read(int fd, char *buffer, int size);
void my_write(int fd, char *buffer, int size);

#endif
