/*
This is the main part of the file system lab with all the functions we need as well as some others.
*/

#include "myfileSys.h"

/* Maps the drive by using mmap function (taken from Professor Fiore)*/
void map()
{
	int i;

	/* Opens and creates a file descriptor for 2MB drive */
	if((drivePointer=open("Drive2MB", O_RDWR)) == -1)
	{
		fprintf(stderr, "error open: %s\n", strerror(errno));
		exit(0);
	}

  	/* By using the mmap function the 2MB drive is mapped so it will be easier to access using FAT Table */
	if((fatMap=mmap(NULL, FAT_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, drivePointer, 0)) == MAP_FAILED) 
	{
		fprintf(stderr, "error fat mapping input: %s\n", strerror(errno));
		exit(0);
	}

	// Maps 2MB drive for data sector
	if((dataMap=mmap(NULL, DATA_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, drivePointer, 4096)) == MAP_FAILED) 
	{
		fprintf(stderr, "error data mapping input: %s\n", strerror(errno));
		exit(0);
	}

	// Initialize all FAT entries to -2
	for(i=0; i<FAT_SIZE; i++)
	{
		fatMap[i] = -2;
	}

	// initialize data map
	for(i=0; i<DATA_SIZE; i++)
	{
		dataMap[i] = 0;
	}
}

// Unmaps drive
void unmap()
{
	// Unmap FAT
    if (munmap(fatMap, FAT_SIZE) == -1) 
    {
        close(drivePointer);
        printf("Error mmap: %s\n", strerror(errno));
		exit(0);
    }

    // Unmap data sector
    if (munmap(dataMap, DATA_SIZE) == -1) 
    {
        close(drivePointer);
        printf("Error mmap: %s\n", strerror(errno));
		exit(0);
    }

    // closes file descriptor
    close(drivePointer);
}



/* initializes root to be always the first 2 Fat entries */
void initRoot()
{
	int fat = 0;

	// Save First 2 FAT Entries for Root Directory 
	fatMap[0] = 1;
	fatMap[1] = -1;

	// Initialize first 2 groups
	initDir(0);
	initDir(1);
}

/* Finds first empty FAT entry */
int findFreeFat()
{
	int i;
	/*Loops through fat and returns first empty Fat signaled by a value of -2*/
	for(i=DATA_START; i<FAT_SIZE; i++)
	{
		if(fatMap[i] == -2)
		{
			return i;
		}
	}

	//if free block not found
	return -2;
}

/* Checks if there is a spot to put file number/name in a directory and if so returns index spot */
int findFreeDir(int index)
{
	int i, end;
	struct dirEntry file;
	end = index + GROUP_LENGTH;

	for(i=index; i<end; i=i+DIR_ENTRY_SIZE)
	{
		memcpy(&file, &dataMap[i], sizeof(struct dirEntry));

		if(file.fatIndex == -1)
		{
			return i;
		}
	}
	return -1;
}

/* Finds FAT location of a specific directory by going through all files by first starting at root directory and going deeper until a file where isDir is set to 1 indicating it is a directory and the file name matches name looking for */
int searchDir(char *dirName, int start)
{
	int i, end, index, holder;
	struct dirEntry file;
	char *name;

	end = start + GROUP_LENGTH;
	if (start == 0)
	{
		end = start + ROOT_SIZE;
	}
	
	// Checks each directory entry
	for(i=start; i<end; i=i+DIR_ENTRY_SIZE){
		memcpy(&file, &dataMap[i], sizeof(struct dirEntry));
		index = file.fatIndex;

		// For empty entries
		if(index <= 0)
		{
			continue;
		}

		// If file is a directory
		if(file.isDir==1)
		{
			name = file.name;

			// Returns FAT index if found
			if (strcmp(dirName, name)==0)
			{
				return index;
			}

			// Searchs for groups in FAT linked list
			holder=fatMap[index];
			while(holder!= -1)
			{
				index = searchDir(dirName, holder*GROUP_LENGTH);
				if(index >= 0)
				{
					return index;
				}
				holder = fatMap[holder];
			}

			// Recursively checks through each subdirectory
			index = searchDir(dirName, index*GROUP_LENGTH);
			if(index >= 0)
			{
				return index;
			}
		}
	}

	return -2;
}

/* Returns the location of a file in the FAT Table by finding which directory it is in and returning FAT Table entry if found */
int searchFile(char *fileName, int start)
	{
	int i, end, index, holder;
	struct dirEntry file;
	char *name;

	end = start + GROUP_LENGTH;
	if (start == 0)
	{
		end = start + ROOT_SIZE;
	}
	
	// Checks each directory entry
	for(i=start; i<end; i=i+DIR_ENTRY_SIZE)
	{
		
		memcpy(&file, &dataMap[i], sizeof(struct dirEntry));
		index = file.fatIndex;
		name = file.name;

		// For empty entries
		if(index <= 0)
		{
			continue;
		}

		// Returns FAT index if found
		if (strcmp(fileName, name)==0)
		{
			if(file.isDir==0)
			{
				return index;
			}
			else if (file.isDir==1)
			{
				return -3;
			}
		}
		//Checks if the file is not a directory
		if(file.isDir != 1)
		{
			continue;
		}

		// Search's for groups in FAT linked list
		holder=fatMap[index];
		while(holder!= -1)
		{
			index = searchFile(fileName, holder*GROUP_LENGTH);
			if(index >= 0)
			{
				return index;
			}
			holder = fatMap[holder];
		}

		// Recursively checks in subdirectory
		index = searchFile(fileName, index*GROUP_LENGTH);
		if(index >= 0)
		{
			return index;
		}
	}

	return -2;
}

/* Either creates a file or a directory in given directory */ 
void my_create(char *fileName, char *dir, char *ext, int isDir)
{
	int i, freeSpot, dirIndex, fatIndex, dataIndex, group;
	struct dirEntry newFile;

	// Finds FAT Index of correct directory, default directory is root
	if((dirIndex=searchDir(fileName, ROOT_INDEX))== FAT_EOF)
	{
		dirIndex = 0;
	}
	group = dirIndex * 512;

	// Find empty dir entry
	if((dataIndex=findFreeDir(group)) == -1) 
	{

		if((freeSpot=findFreeFat()) == -2)
		{
			printf("Error: Out of FAT SPACE\n");
		}

		//Allocate more space for directory
		fatMap[dirIndex] = freeSpot;
		fatMap[freeSpot] = -1;
		
		initDir(freeSpot);
		group = freeSpot * 512;
		dataIndex=findFreeDir(group);
	}

	// find free fat index
	if((fatIndex=findFreeFat()) == -2)
	{
		printf("Error: Out of FAT SPACE\n");
		exit(0);
	}

	/* Initialize file entry with appropriate metadata */
	newFile.name = fileName;
	newFile.ext = ext;
	newFile.fatIndex = fatIndex;
	newFile.isDir = isDir;
	newFile.time = time(0);
	fatMap[fatIndex] = -1;

	// add file to directory
	memcpy(&dataMap[dataIndex], &newFile, sizeof(struct dirEntry));

	// if directory, initialize directory
	if(isDir == 1)
	{
		initDir(fatIndex);
	}
}

/* Finds file & creates file descriptor to first chunk of data */
int my_open(char *fileName)
{
	int i, fat, index;

	// find fat index of file
	fat = searchFile(fileName, 0);
	if(fat == -2)
	{
		printf("Error: no file found or is directory\n");
		exit(0);
	}
	
	// return int starting at group location
	return fat*GROUP_LENGTH;
}

/* Closes file pointer to first group */
void my_close(int* fd){
	// Closes pointer by setting int to -1
	*fd = -1;
}

/* Remove a file's directory entry and initialize data in group and FAT to be identified as empty */
int my_delete(char *fileName)
{
	int i, fat, index, start, end, entrySize, holder, location;
	struct dirEntry empty;

	// Initialize empty struct directory entries
	entrySize = sizeof(struct dirEntry);
	empty.name = "";
	empty.ext = "";
	empty.fatIndex = -1;
	empty.isDir = -1;
	empty.time = 0;

	fat = searchFile(fileName, 0);
	if(fat == -2)
	{
		printf("Error: file not found\n");
		exit(0);
	}
	else if(fat == -3)
	{
		printf("Error: can't delete directory unless it is empty\n");
	}


	// Remove directory entry
	location = searchFileEntry(fat, 0);
	memcpy(&dataMap[location], &empty, entrySize);

	// Iterate over file in FAT table
	index = fatMap[fat];
	fatMap[fat] = -2;
	while(index != -1)
	{

		// Zero out one group so that it is signaled as empty
		start = index*512;
		end = start + GROUP_LENGTH;
		for(i=start; i< end; i++)
		{
			dataMap[i] = 0;
		}
		
		holder = fatMap[index];
		fatMap[holder] = -2;
		index = holder;
	}
	return 1;
}

/* Read File 
	Search for file, Go to FAT Table, read from data group, when at end, check if there
	is another data group in FAT Table to continue reading from if data is more
*/
void my_read(int fd, char *buffer, int count)
{
	int i, fat, start, end, data, dataIndex, hold;
	fat = fd/GROUP_LENGTH;

	// Decides if data fits into greater than one group
	if(count <= GROUP_LENGTH)
	{
		end = fd+count;
		hold = 0;
	}
	else
	{
		end = fd+GROUP_LENGTH;
		hold = 1;
	}

	// Stores up to size n bytes into buffer
	for(i=fd; i<end; i++)
	{
		*buffer = dataMap[i];
		buffer++;
	}

	// If size is bigger than 1 group, recursively write to next group
	if(hold)
	{	
		dataIndex = dataMap[fat*GROUP_LENGTH];

		// recursively writes to next group
		count = count - GROUP_LENGTH;
		my_read(dataIndex, buffer, count);
	}
}

/* Write File 
	Search for file, Go to FAT Table, write to data group, when at end, find free
	another data group in FAT Table, write to new data group, update FAT
*/
void my_write(int fd, char *buffer, int size){
	int i, fat, freeSpot, end, hold, dataIndex;
	fat = fd/GROUP_LENGTH;

	// Decides if data fits into greater than one group
	if(size <= GROUP_LENGTH)
	{
		end = fd+size;
		hold = 0;
	}
	else
	{
		end = fd+GROUP_LENGTH;
		hold = 1;
	}
	// writes data to 1 group
	for(i=fd; i<end; i++)
	{
		dataMap[i] = *buffer;
		buffer++;
	}

	// If size is bigger than 1 group, recursively write to next group
	if(hold){
		// Determines if another group is already allocated
		if(fatMap[fat] >= -1)
		{
			dataIndex = dataMap[fat*GROUP_LENGTH];
		}
		else
		{
			// allocates another group to file
			freeSpot = findFreeFat();
			fatMap[fat] = freeSpot;
			fatMap[freeSpot] = -1;
			dataIndex = dataMap[freeSpot*GROUP_LENGTH];
		}

		// recursively writes to next group
		size = size - GROUP_LENGTH;
		my_write(dataIndex, buffer, size);
	}
}
/* initialize a directory by setting all struct variables to appropriate values to signify it is empty*/ 
void initDir(int index)
{
	int i, entrySize, begin, end;
	struct dirEntry empty;

	// Initialize empty struct directory entries
	entrySize = sizeof(struct dirEntry);
	empty.name = "";
	empty.ext = "";
	empty.fatIndex = -1;
	empty.isDir = -1;
	empty.time = 0;

	// Fills a group with empty directory structs
	begin = index * 512;
	end = begin+GROUP_LENGTH;
	for(i=begin; i<end; i=i+DIR_ENTRY_SIZE)
	{
		memcpy(&dataMap[i], &empty, entrySize);
	}
}

/* returns file entry location given a its fat index */
int searchFileEntry(int fatIndex, int start)
{
	int i, end, index, holder;
	struct dirEntry file;
	char *name;

	end = start + GROUP_LENGTH;
	if (start == 0)
	{
		end = start + ROOT_SIZE;
	}
	
	// Checks each directory entry
	for(i=start; i<end; i=i+DIR_ENTRY_SIZE)
	{
		memcpy(&file, &dataMap[i], sizeof(struct dirEntry));
		index = file.fatIndex;
		name = file.name;

		// For empty entries
		if(index <= 0)
		{
			continue;
		}

		// Returns FAT index if found
		if (index ==fatIndex)
		{
			if(file.isDir==0)
			{
				return i;
			}
			else if (file.isDir==1)
			{
				return -1;
			}
		}

		if(file.isDir != 1)
		{
			continue;
		}

		// Searchs for groups in FAT linked list
		holder=fatMap[index];
		while(holder!= -1)
		{
			i = searchFileEntry(fatIndex, holder*GROUP_LENGTH);
			if(i >= 0)
			{
				return i;
			}
			holder = fatMap[holder];
		}

		// Recursively checks in subdirectory
		i = searchFileEntry(fatIndex, index*GROUP_LENGTH);
		if(index >= 0)
		{
			return index;
		}
	}

	return -1;
}




