/* Program that works as a FTP client 
 * there are different commands of ftp that user can choose and see a result
 * Authors: Nazym Turysbek, Aru Omarali
 * Date: 16.03.2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sendfile.h>

void error(const char *msg)
{
  perror(msg);
  exit(0);
}

int main(int argc, char *argv[])
{   
  int sockfd, portnumber, n,m, message;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  struct stat obj;
  char buf_user[255], buf_pass[255];
  int choice,k,size,status,filehandle;
  char buf[100], command[5],filename[20], *f;
  int i = 1, cnt = 0;
  
  if(argc < 3)
  { 
    fprintf(stderr, "usage %s hostname port\n", argv[0]);
    exit(0);
  }
  //Задаем номер порта
  portnumber = atoi(argv[2]);
  //Создаем сокет, domain, type, protocol
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if(sockfd < 0)
  {
    error("Error opening Socket");
  }
  
  // Конвертирует имя хоста в IP адрес
  server = gethostbyname(argv[1]);
  if(server == NULL)
  {
    fprintf(stderr, "Error, no such host");
  } 
  
  //Указываем тип сокета Интернет
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;

  //Указываем адрес IP сокета
  bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);

  //Указываем порт  host to network short
  //перевод целого короткого числа из порядка байт, принятого на компьютере, в сетевой порядок байт
  serv_addr.sin_port = htons(portnumber);  //адрес порта в ip имеет размер 2 байта  
  //Устанавливаем соединение
  if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
  {
    error("Connection Failed");
  }

  while(1)
  {
    if(cnt == 0)
    {    

      printf("_____________________\n\n");
      printf("     FTP client \n");
      printf("_____________________ \n\n");
  	  printf("        Username : ");
  	  bzero(buf_user, 255);
  	  fgets(buf_user, 255, stdin);
      // Отправляем данные
  	  n = write(sockfd, buf_user, strlen(buf_user));

  	  printf("        Password : ");
  	  bzero(buf_pass, 255);
  	  fgets(buf_pass, 255, stdin);
      // Отправляем данные
  	  m = write(sockfd, buf_pass, strlen(buf_pass));

  	  if(n < 0 && m < 0)
      {
  	      error("Error on writing");
      }
      //Обнуляем буфер
      bzero(buf_pass, 255);
      recv(sockfd, buf_pass, 100,0);
      printf("Access : %s", buf_pass);
      cnt++;
    }  

    if(strcmp(buf_pass, "You have logged in successfully")==0)
    {
      printf("\n ftp>Enter a choice: \n1- get\n2- put\n3- ls\n4- cd\n5- quit\n");
      scanf("%d", &choice);

      switch(choice)
      {
        case 1:
          printf("ftp> Enter filename to get: ");
          scanf("%s", filename);
          strcpy(buf,"get ");
          strcat(buf, filename);
          printf("File Name: %s\n",filename);
          printf("Received: %d Mb",(size/1024));
          send(sockfd, buf, 100, 0);
          recv(sockfd, &size, sizeof(int), 0);
          if(!size)
          {
            printf("No such file on the remote directory\n\n");
            break;
          }
          f = malloc(size);
          recv(sockfd,f, size, 0);
          while(1)
          {
            filehandle = open(filename, O_CREAT | O_EXCL | O_WRONLY | O_RDONLY,0666);
            if(filehandle == -1)
            {
              sprintf(filename + strlen(filename), "%d", i);
            }
            else
              break;
            }
            write(filehandle, f, size);
            close(filehandle);
            strcpy(buf, "cat ");//open the file
            strcat(buf, filename);//display on the screen
            //system(buf);
          break;

        case 2:
          printf("Enter filename to put to server: ");
          scanf("%s", filename);
          filehandle = open(filename, O_RDONLY);

          if(filehandle == -1)
          {
            printf("There is no such file!\n\n");
            break;
          }

          strcpy(buf, "put ");
          strcat(buf, filename);
          send(sockfd, buf, 100, 0);
          //получаем информацию о файле файловый дескриптор, структура куда мы сохраняем информацию о файле
          stat(filename, &obj);
          //определяем размер файла
          size = obj.st_size;
          //отправка данных
          send(sockfd, &size, sizeof(int), 0);
          sendfile(sockfd, filehandle, NULL, size);
          //чтение данных из сокета
          recv(sockfd, &status, sizeof(int), 0);
          if(status)
            printf("File is stored successfully!\n");
          else
            printf("Failed!\n");
            break;
      
        case 3:
          strcpy(buf, "ls");
          send(sockfd, buf, 100, 0);
          recv(sockfd, &size, sizeof(int), 0);
          f = malloc(size);
          recv(sockfd, f, size, 0);
          filehandle = open("content.txt", O_RDWR | O_CREAT, S_IWUSR | S_IRUSR, 0066);
          write(filehandle, f, size);
          close(filehandle);
          printf("The directory contains: \n");
          system("cat content.txt");
          break;
    
        case 4:
          strcpy(buf, "cd");
          printf("Path: ");
          scanf("%s", buf + 3);
          send(sockfd, buf, 100, 0);
          recv(sockfd, &status, sizeof(int), 0);

          if(status)
            printf("Directory successfully changed\n");
          else
            printf("Failed!\n");
          break;
     
        case 5:
          strcpy(buf,"quit ");
          send(sockfd, buf, 100, 0);
          recv(sockfd, &status, 100, 0);

        printf("------------------------------------------------\n\n");
      	  if(status)
      	  {
    	      printf("Server closed");
    	      exit(0);
          }
          
          printf("Server failed to close connection\n");       
          break;      
      }
    }

    else 
    {
      printf("\nTry it again");
      break;
    }
  }

  close(sockfd);  
  return 0;
}
