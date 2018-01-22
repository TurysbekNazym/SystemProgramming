#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
int main(){

  int len = 1000;
  char str[100];
  char s[100];
  char buf[len];
  int  b;
  ssize_t nr;
  ssize_t wr;
  int fd,fd1;

  printf("Hello! Enter the two value:\n");
  scanf("%s %s", str,s);
  if(strcmp(s,str)==0){
     printf("Not allowed to the same file \n");
  }
  else
 {
  fd =  open(str,O_CREAT | O_RDONLY);
  fd1 = open(s,O_CREAT | O_WRONLY);

  if(fd == -1){
    printf("Error Number %d\n",errno);
    perror("Program"); 
    exit(1);
  }

  if(fd1 == -1){
    printf("Error Number %d\n",errno);
    perror("Program"); 
    exit(1);
  }
  
  //nr = read(fd, buf, len);
  //if(nr == -1){
    //if(errno == EINTR)
      //printf("goto");
    //if(errno == EAGAIN)
      //return 0;
   //}
  while(len != 0 && (nr = read (fd, buf, len))!=0){
       if(nr == -1){
            if(errno == EINTR)
                  continue;
             perror ("read");
             break;
       }
      len -=nr; 
      b +=nr;

  }

   wr = write (fd1,buf,strlen(buf));
  if(wr == -1)
    printf("write error");

   close(fd);
   close(fd1);
 }
  return 0;

}
