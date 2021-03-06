/* Program that works as a FTP server 
 * Server accepts multiclients and receives, sends responds for their commands.
 * Authors: Nazym Turysbek, Aru Omarali
 * Date: 16.03.2018
*/
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>

struct sockaddr_in cli_addr;
struct stat obj;// contains file information example size
char buf_user[255];
char buf_pass[255];
int n,m,j=0; 
char *users[]={"Aru","Nazym", "Assem"};
char *passwords[]={"12345","123","111"};
char message[50]="You have logged in successfully";
char message2[50]="Your login or password incorrect";
int a,b,k, i, size, len, c,filehandle, cnt = 0;
char buf[100], command[5], filename[20],cmd[20];

void error(const char *msg)//print error  message
{
  perror(msg);
  exit(1);
}

void Authorizhation(int newsockfd)//fuction for authorization
{
  bzero(buf_user, 20);
  n = read(newsockfd, buf_user, 20);
  bzero(buf_pass, 20);
  m = read(newsockfd, buf_pass, 20);
  
   if(n < 0 && m < 0)
    error("Error on reading");
    printf("Username: %s\n", buf_user);//message from client
    printf("Password: %s\n", buf_pass);//message from client
    int i = 0;
    int cl = 0;
    for(i = 0; i < 3; i++ )
    {
      
      a = strncmp(users[i],buf_user,strlen(users[i]));
      b = strncmp(passwords[i],buf_pass, strlen(passwords[i]));
      //printf("%s\n", users[i]);
      //printf("%s\n", passwords[i]);
       printf("%d\n", a);
       printf("%d\n", b);
       if(a==0 && b==0){  
           cl = 1;
             printf("%d\n", cl);
       }
    }
    if(cl==1)
      {
        send(newsockfd, message, strlen(message),0);
        
      }   
      else
      {
        send(newsockfd, message2, strlen(message2),0);    
      }
}

void* SendFileToClient(int *arg)//thread accept parametr which is id of client
{
  int newsockfd = (int)*arg;
  printf("Connection accepted and id: %d\n",newsockfd);
  printf("Hello\n");
  Authorizhation(newsockfd);
  
  while(1)
  {
    bzero(buf, 100);
    recv(newsockfd, buf, 100, 0);
    printf("%s\n", buf);
    sscanf(buf, "%s", command);// divide and get command

    if(strcmp(command,"get")==0)//get
    {
  	  sscanf(buf, "%s%s", cmd, filename);
  	  stat(filename, &obj);
  	  filehandle = open(filename, O_RDONLY);
  	  size = obj.st_size;

  	  if(filehandle == -1)
            size = 0;
         send(newsockfd, &size, sizeof(int), 0);
  	  
      if(size)
        sendfile(newsockfd, filehandle, NULL, size);
      
    }
   
    else if(strcmp(command,"quit") == 0)
    {
       printf("FTP server quitting..\n");
       int i = 1;
       send(newsockfd, &i, sizeof(int), 0);  
       exit(0); 
    }

    else if(strcmp(command, "ls") == 0)
    {
    	system("ls >content.txt");
    	i = 0;
    	stat("content.txt",&obj);
    	size = obj.st_size;
    	send(newsockfd, &size, sizeof(int), 0);
    	filehandle = open("content.txt", O_RDONLY);
    	sendfile(newsockfd,filehandle,NULL,size);
    }

    else if(strcmp(command, "put") == 0)
    {
  	  int c = 0, len;
  	  char *f;
  	  sscanf(buf+strlen(command), "%s", filename);
  	  recv(newsockfd, &size, sizeof(int), 0);
  	  i = 1;

  	  while(1)
      {
        filehandle = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0666);

        if(filehandle == -1)
    		{
    		  sprintf(filename + strlen(filename), "%d", i);
    		}
        else
  		    break;
      }

  	  f = malloc(size);
  	  recv(newsockfd, f, size, 0);
  	  c = write(filehandle, f, size);
  	  close(filehandle);
  	  send(newsockfd, &c, sizeof(int), 0);
    }
      
    else if(strcmp(command, "cd") == 0)
    {
      if(chdir(buf+3) == 0)
      {
        c = 1;
      }
      else
        c = 0;
      
      send(newsockfd, &c, sizeof(int), 0);
    }
  }
  
  close(newsockfd);
  pthread_exit(NULL);
  return;
}

int main(int argc, char *argv[])
{
  pthread_t tid; 
  int sockfd,newsockfd,check,portnumber;
  struct sockaddr_in serv_addr;
  socklen_t clilen;
  sockfd = socket(AF_INET, SOCK_STREAM, 0); // CREATING SOCKET

  if(sockfd < 0)
  {
    error("Error opening Socket");
  }

  bzero((char *)&serv_addr, sizeof(serv_addr)); //make zero
  portnumber = 2100;
  serv_addr.sin_family = AF_INET;//ipv4
  serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //127.0.0.1 10.42.0.196 172.16.86.121 
  serv_addr.sin_port = htons(portnumber);
   
  if(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
    error("Binding failed");
  }

  if(listen(sockfd, 10) == -1)
  {
    printf("Failed to listen\n");
    return -1;
  }

  while(1)
  {   
    clilen = sizeof(cli_addr);
    printf("Waiting...\n");   
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

    if(newsockfd < 0)
        printf("Error on Accept");
    printf("Connected to Clent: %s:%d\n",inet_ntoa(cli_addr.sin_addr),ntohs(cli_addr.sin_port));
    check = pthread_create(&tid, NULL, &SendFileToClient, &newsockfd);
    
    if (check!= 0)
      printf("\ncan't create thread :[%s]", strerror(check));
      j++;
  }

  close(newsockfd);
  close(sockfd);
}
