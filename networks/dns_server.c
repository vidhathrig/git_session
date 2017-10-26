#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#define BUF_SIZE 120


struct Node
{
	char domainName[20];	//host name
	char ipAddr[65];		//IP address of the host
	struct Node *next;
}l;

typedef struct Node *nodePointer;	// nodePointer points to a structure of type linkedList

//Function prototypes
void RespondClient(int socket,char clientIp[],struct sockaddr_in clientAddr,int recvMsgSize);
void readInput(char *, nodePointer *);
void * searchDomain(char *, nodePointer *);

//Global variables
char fileLocation[100];	//Location of file which contains ip addresses of domain
char buffer[BUF_SIZE];			
nodePointer head = NULL;

int main(int argc,char *argv[])
{
	int server_port = 1870;		//Server port
	char line[1024];
	int sock_fd;
	struct sockaddr_in serverAddr;  //server address
	struct sockaddr_in clientAddr;	//Client address
	int clientLen;					//Length of client addr
	int recvMsgSize;				//Size of client message

	strcpy(fileLocation,argv[1]);	//Give location of ip_map.txt file

	printf("Server on port number %d\n",server_port );

	//Read the ip_map.txt file and store contents in linked list in the form of domain name and ip addresses in each node
	FILE *fp;
	fp = fopen("ip_map.txt","r");
	if(fp == NULL)
	{
		return -1;
	}
	else 
	{
		while(fgets(line,sizeof(line),fp))
		{
			readInput(line, &head);
		}
	}

	//SOCKET CREATION
	if((sock_fd = socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP))<0)		
	{
		perror("Socket creation failed\n");
		exit(1);
	}
	else
		printf("\nSocket creation successful\n");

	//SERVER ADDRESS STRUCTURE
	memset(&serverAddr, 0, sizeof(serverAddr)); 		//initialize all fields of serverAddr to zero
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);		//Address of the server
    serverAddr.sin_port = htons(server_port);

  //  printf("Server address :%s",inet_ntoa(serverAddr));

    //BIND
    if(bind(sock_fd,(struct sockaddr *) &serverAddr,sizeof(serverAddr))<0)
    {
    	perror("Bind failed\n");
    	exit(1);
    }
    else
    	printf("\nBind successful\n");

    int i=0;

    //Runs forever
    for(;;)
    {
    	clientLen = sizeof(clientAddr);
    	//Blocks until recieve a message from client
    	if((recvMsgSize = recvfrom(sock_fd,buffer,BUF_SIZE,0,(struct sockaddr *) &clientAddr, &clientLen))<0)//recieve the request from client
    	{
    		printf("Error recieving message from client\n");
    		exit(1);
    	}
    	if (recvMsgSize > 0)
    	{  
            RespondClient(sock_fd,inet_ntoa(clientAddr.sin_addr),clientAddr, recvMsgSize);
        }
    }

    return 0;
}

void RespondClient(int socket,char clientIp[],struct sockaddr_in clientAddr,int recvMsgSize)
{
	printf("\nServing client %s\n",clientIp);
	char *word;
	buffer[recvMsgSize]='\0';
	char *ip_pointer;


	word = buffer;
	printf("\nClient searching for ip address of %s\n",word);
	if((ip_pointer = searchDomain(word,&head))!=NULL)			//Search for the given domain in linked list
	{
		strcpy(buffer,ip_pointer);
		buffer[strlen(ip_pointer)] = '\0';
	}

	else
	{
		struct hostent *hp = gethostbyname(buffer);			//If not found in linked list try using gethostbyname()
		if(hp == NULL)
			strcpy(buffer,"Domain not found");
		else
		{
			unsigned int i=0;
        
        while ( hp -> h_addr_list[i] != NULL) {      
            
                strcpy(buffer, inet_ntoa( *( struct in_addr*)( hp -> h_addr_list[i]))); 
              
            i++;
		}
	}
}
	//Send response to the client
	if(sendto(socket,buffer,BUF_SIZE,0,(struct sockaddr *) &clientAddr,sizeof(clientAddr)) != BUF_SIZE)
	{
		printf("Sent a different number of bytes\n");
		exit(1);
	}
	else
	{
		printf("\nsent response to client\n");
	}
}


void readInput(char line[],nodePointer *first)
{
	char *word;
	nodePointer current = malloc(sizeof(l));		//Allocating memory to current pointer
	if((*first)==NULL)
	{
		*first = current;
		(*first)->next = NULL;
	}
	else
	{
		(*current).next = (*first);
		(*first) = current;
	}

	word = strtok(line," ");						//Breaking domin name and ip addresses and inserting them in linked list
	strcpy((*first)->domainName,word);
	word = strtok(NULL,"\n");
	strcpy((*first)->ipAddr,word);
}

//Function to search for a given domain name
void * searchDomain(char *domain,nodePointer *first)
{
	char temp[100];
    int flag=0;
    nodePointer current = (*first);
    while(current !=NULL)
    {
    	if(strcmp(current->domainName,domain)==0)	//Compare the host sent by the client with existing data
    	{
    		printf("\nMatch found\n");
    		flag = 1;
    		break;
    	}
    	current = current->next;
    }

    if(flag == 0)
    		return NULL;		
    else
    	return (char *)current->ipAddr; 			//return the ip address if match is found
}