#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <pthread.h>
#include <semaphore.h>

#define DEFAULT_PORT "2000"
#define DEFAULT_DICTIONARY "words.txt"


//Change as needed
#define BACKLOG 10
#define EXIT_USAGE_ERROR 1
#define EXIT_GETADDRINFO_ERROR 2
#define EXIT_BIND_FAILURE 3
#define EXIT_LISTEN_FAILURE 4
#define MAX_LINE 64
#define NUM_WORKERS 2
#define SIZE_OF_SOCKET 1
#define SIZE_OF_QUEUE 2

void *worker(void *);
int service(char ** , int );
int wordInDictionary(char *, char **);
char **makeDictionary(char *);
int countWords(FILE *);
void makeQueue();
void makeSocket(int );
int removeSocket();
int getListenfd(char *);
ssize_t readLine(int , void * , size_t );

/*Struture for Queue*/
typedef struct {
	int *sockets;
	int front;
	int end;
	int maxCap;
	int size;
	sem_t * mutex;
	sem_t * slots;
	sem_t * items;
} sbuf_t;
sbuf_t queue;

int main(int argc, char **argv){
	int i;
	int listenfd; 			/*listen socket descriptor*/
	int connectedfd; 		/* connected socket descriptor*/
	char clientID[MAX_LINE];
	char clientPort[MAX_LINE];
	char **dict; 			/* Dictionary*/
	char *port;			/*Port */
	struct sockaddr_storage client_addr;
    	socklen_t client_addr_size;
	ssize_t bytes_read;char line[MAX_LINE];
	pthread_t wthread[NUM_WORKERS];

	/*Initalize the struct meaning intialize semaphores values*/
	makeQueue();
	
	/*If there are 2 arguments use command line port and dictionary*/ 
	if(argc==3)
	{
		port = argv[1];
		dict=makeDictionary(argv[2]); 
	}
	/*If only 1 argument it is port*/
	else if(argc== 2)
	{
		port= argv[1];
		dict=makeDictionary(DEFAULT_DICTIONARY);
	}
	/*If no command line arguments use defaults*/
	else
	{
		port=DEFAULT_PORT;
		dict=makeDictionary(DEFAULT_DICTIONARY);
	}

	/*Server listener*/
	listenfd = getListenfd(port);

	/* Make workers */
	for(i=0; i<NUM_WORKERS; i++)
	{
		if(pthread_create(&wthread[i], NULL, worker, (void*)dict)) 
		{
    		perror("Error creating thread");
    		}
	}
	
	/*Continuously accept clients*/
	while(1)
	{
	client_addr_size=sizeof(client_addr);
       
	 /*Accept client and add them onto queue*/
		if ((connectedfd=accept(listenfd, (struct sockaddr*)&client_addr, &client_addr_size))==-1) 
		 {
       		  fprintf(stderr, "accept error\n");
      		  continue;
   		 }
        	 if (getnameinfo((struct sockaddr*)&client_addr, client_addr_size,
		    clientID, MAX_LINE, clientPort, MAX_LINE, 0)!=0)
		 {
     		 fprintf(stderr, "error getting name information about client\n");
    		}
	/*If client is accepted properly will print accepted with specfic address and port*/ 
		else {
      			printf("accepted connection from %s:%s\n", clientID, clientPort);
   		 }

	 /*Add the socket where client and socket and communicate to queue*/
		makeSocket(connectedfd); 
   	 }
	

	return 0;
		

}

/* given a port number or service as string, returns a
   descriptor that we can pass to accept()   ----Taken from Professor Fiore's Code */
int getListenfd(char *port) {
  int listenfd, status;
  struct addrinfo hints, *res, *p;

  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM; /* TCP */
  hints.ai_family = AF_INET;	   /* IPv4 */

  if ((status = getaddrinfo(NULL, port, &hints, &res)) != 0) {
    fprintf(stderr, "getaddrinfo error %s\n", gai_strerror(status));
    exit(EXIT_GETADDRINFO_ERROR);
  }

  /* try to bind to the first available address/port in the list.
     if we fail, try the next one. */
  for(p = res;p != NULL; p = p->ai_next) {
    if ((listenfd=socket(p->ai_family, p->ai_socktype, p->ai_protocol))<0) {
      continue;
    }

    if (bind(listenfd, p->ai_addr, p->ai_addrlen)==0) {
      break;
    }
  }
  freeaddrinfo(res);
  if (p==NULL) {
    exit(EXIT_BIND_FAILURE);
  }

  if (listen(listenfd, BACKLOG)<0) {
    close(listenfd);
    exit(EXIT_LISTEN_FAILURE);
  }
  return listenfd;
}

/* FROM KERRISK 

   Read characters from 'fd' until a newline is encountered. If a newline
   character is not encountered in the first (n - 1) bytes, then the excess
   characters are discarded. The returned string placed in 'buf' is
   null-terminated and includes the newline character if it was read in the
   first (n - 1) bytes. The function return value is the number of bytes
   placed in buffer (which includes the newline character if encountered,
   but excludes the terminating null byte). */

ssize_t readLine(int fd, void *buffer, size_t n) {
  ssize_t numRead;                    /* # of bytes fetched by last read() */
  size_t totRead;                     /* Total bytes read so far */
  char *buf;
  char ch;

  if (n <= 0 || buffer == NULL) {
    errno = EINVAL;
    return -1;
  }

  buf = buffer;                       /* No pointer arithmetic on "void *" */

  totRead = 0;
  for (;;) {
    numRead = read(fd, &ch, 1);

    if (numRead == -1) {
      if (errno == EINTR)         /* Interrupted --> restart read() */
	continue;
      else
	return -1;              /* Some other error */

    } else if (numRead == 0) {      /* EOF */
      if (totRead == 0)           /* No bytes read; return 0 */
	return 0;
      else                        /* Some bytes read; add '\0' */
	break;

    } else {                        /* 'numRead' must be 1 if we get here */
      if (totRead < n - 1) {      /* Discard > (n - 1) bytes */
	totRead++;
	*buf++ = ch;
      }

      if (ch == '\n')
	break;
    }
  }

  *buf = '\0';
  return totRead;
}


/* Worker attempts to remove descriptor from queue and services a client once done repeats*/
void *worker(void *arg){
	char **dict = (char **) arg;
	int clientSocket;int x;

	/*Continuously service clients as long as items in queue*/
	while(1){
		/* Will attempt to remove socket from queue if not empty else will block
			until not empty*/
		clientSocket = removeSocket();
		if((x=service(dict, clientSocket))!=0)
		{printf("Error");}
		printf("Connection closed.\n");
		close(clientSocket);	
	}
}

/*Will service individual client in sense of checking if words given is spelled correctly
  if it is in dictionary */

int service(char **dict, int clientSocket) {
	ssize_t bytes_read;
	char line[MAX_LINE];

    while ((bytes_read=readLine(clientSocket, line, MAX_LINE-1))>0) {
	    /*Will check if word in dictionary and concat OK or MISSPELLED*/
	    if (wordInDictionary(line, dict)){
	      strcat(line, " OK\n");
	    }
	    else {
	      strcat(line, " MISSPELLED\n");
	    }

	  /* Can write to server side 
	    printf("%s\n", line);
	   */
	 /* Writes to client if spelled correctly or not*/  
	  write(clientSocket, line, strlen(line));
    }
  return 0;
}

/* Checks if word is in dictionary or not */
int wordInDictionary(char *word, char **dict) {
    int i; 
    int Len;
    int j = 0;

    Len = strlen(word);
    if (Len - 1 >= 0)
	    word[Len-1] = '\0';
    Len = strlen(word);

	while(dict[j] != NULL){
		/* Don't bother comparing word if length's are not the same */
	    if (Len != strlen(dict[j])){
	    	j++;
	     	continue;
	    }
	    if(strcmp(dict[j], word) == 0){
	    	return 1;
	    }
	    j++;
	}

  return 0;
}


/*Makes the queue and intializes the semaphores and variables correctly*/
void makeQueue(){
	int i;
	queue.sockets = calloc(SIZE_OF_QUEUE, sizeof(int));
	queue.maxCap = SIZE_OF_QUEUE;			/*Capacity of Queue*/
	queue.size = 0;		
	queue.front = queue.end = 0;
	/*Initalizes semaphores since I have a Mac using open*/
	/*mutex initalized to 1, num of slots to size of queue, and items in queue to 0*/ 
	if((queue.mutex= sem_open("mysemaphore3", O_CREAT,S_IRUSR |S_IWUSR , 1))==SEM_FAILED)
		printf("Error in creating semaphore");
	if((queue.slots= sem_open("mysemaphore1", O_CREAT, S_IRUSR |S_IWUSR ,SIZE_OF_QUEUE))==SEM_FAILED)
		printf("Error in creating semaphore");
	if((queue.items= sem_open("mysemaphore2", O_CREAT,S_IRUSR |S_IWUSR , 0))==SEM_FAILED)
		printf("Error in creating semaphore");

	/* If using non-Mac OS correct way to initalize semaphores 
	sem_init(&queue.mutex, 0, 1);
	sem_init(&queue.slots, 0, SIZE_OF_QUEUE);
	sem_init(&queue.items, 0, 0);
	*/
}

/*Will remove a client's descriptors off the queue if not empty and return it*/
int removeSocket(){
	int clientSocket;
	/*If no items in queue (aka queue is empty) then will go to sleep*/
	sem_wait(queue.items);
	/*If we get here then queue is not empty and now try to get the lock*/
	sem_wait(queue.mutex);
	clientSocket = queue.sockets[ (++queue.front) % (queue.maxCap)];
	/*Release the lock*/
	sem_post(queue.mutex);
	/*Increase slots and therfore wakeup any producers*/
	sem_post(queue.slots);
	return clientSocket;
}

/*Will add the descriptor bewteen client and socket onto queue*/
void makeSocket(int clientSocket){
	/*If no slots avaiable (aka queue is full) will go to sleep*/
	sem_wait(queue.slots);
	/*Acquire lock*/
	sem_wait(queue.mutex);
	queue.sockets[ (++queue.end) % (queue.maxCap) ] = clientSocket;
	/*Release lock*/
	sem_post(queue.mutex);
	/*Increase items and therefore wake up any consumers */
	sem_post(queue.items);
}

/*Opens textfile and creates an array of the words which is returned*/
char **makeDictionary(char *dict){
    int length, count=0, i=0;
    char **buf;
    char *c, line[64];
    FILE *fp = fopen(dict, "r");
  
    length = countWords(fp);
    rewind(fp);
    buf = malloc((length+1) *sizeof(char*));

    while ((c=fgets(line, 64, fp)) != NULL){
        buf[i] = strdup(c);
        int ln = strlen(buf[i]);
        if(buf[i][ln-1] == '\n'){
            buf[i][ln-1] = '\0';
        }
        i++;
    }
    buf[length+1] = NULL;
    return buf;
}

/*Counts number of words in the text file*/
int countWords(FILE *fp){
    int count = 0;
    char line[64];
    char *c;

    while ((c=fgets(line, 64, fp)) != NULL){
    	count++;
    }
    return count;
}





































