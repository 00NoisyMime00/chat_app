#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
 
int main(void){

  int sockfd = 0,n = 0;
  char recvBuff[1024];
  char sendBuff[1025];
  char name[] = "anon";
  struct sockaddr_in serv_addr;
  pid_t pid;
 
  memset(recvBuff, '0' ,sizeof(recvBuff));
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0))< 0){
      printf("\n Error : Could not create socket \n");
      return 1;
  }
 
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(5000);
  serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
 
  if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0){

      printf("\n Error : Connect Failed \n");
      return 1;
  }

  write(sockfd, name, strlen(name));

  n = read(sockfd, recvBuff, sizeof(recvBuff)-1);
  recvBuff[n] = 0;

  if(fputs(recvBuff, stdout) == EOF){
    printf("\n Error : Fputs error");
  }

  printf("\n");

  // while((n = read(sockfd, recvBuff, sizeof(recvBuff)-1)) > 0){
  //   recvBuff[n] = 0;
      
  //   if(fputs(recvBuff, stdout) == EOF){
  //     printf("\n Error : Fputs error");
  //   }

  //   printf("\n");
  //   strcpy(sendBuff, "Message from client");
  //   printf("a is hehre");
  //   write(sockfd, sendBuff, strlen(sendBuff));
  //   // printf("b");
  // }
  
  if( n < 0){

    printf("\n Read Error \n");
  }

  pid = fork();

  if (pid < 0)
      perror("ERROR on fork");
  if (pid == 0)  {
    while(1){
      // printf("this- %d\n", connfd);
      char str[1024];
      fgets(str, 1024, stdin);
      printf("sending- %s\n", str);
      write(sockfd, str, strlen(str));
    }
  }
  else{
    while(1){
      n = read(sockfd, recvBuff, sizeof(recvBuff)-1);
      recvBuff[n] = 0;

      if(fputs(recvBuff, stdout) == EOF){
        printf("\n Error : Fputs error");
      }
    }
  }

  printf("closed\n");
  return 0;
}