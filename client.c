#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

pthread_t child;

// Thread function to receive messages from the server
void *recieve_message_thread(void *_args){
  while(1){
    int sockfd = *(int *)_args;
    char str[1024];
    
    fgets(str, 1024, stdin);
    write(sockfd, str, strlen(str));
  }
}
 
 void handler(int sig_num){
   printf("Goodbye, come back again\n");
   exit(0);
 }

int main(int argc, char *argv[]){

  signal(SIGINT, handler);

  int sockfd = 0,n = 0;
  char recvBuff[1024];
  char sendBuff[1025];
  char name[100] = "anon";
  struct sockaddr_un serv_addr;
  pid_t pid;

  if(argc == 2){
    strcpy(name, argv[1]);
  }
  else if(argc > 2){
    printf("Too many arguments supplied!!\n");
    exit(0);
  }
 
  memset(recvBuff, '0' ,sizeof(recvBuff));
  if((sockfd = socket(AF_UNIX, SOCK_STREAM, 0))< 0){
      printf("\n Error : Could not create socket \n");
      return 1;
  }
 
  serv_addr.sun_family = AF_UNIX;
  strcpy(serv_addr.sun_path, "socket");
 
  if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0){

      printf("\n Error : Connect Failed \n");
      return 1;
  }

  write(sockfd, name, strlen(name));

  n = read(sockfd, recvBuff, sizeof(recvBuff)-1);
  recvBuff[n] = 0;


  printf("\r%s", recvBuff);
  fflush(stdout);

  if( n < 0){

    printf("\n Read Error \n");
  }


// get the help menu
  write(sockfd, "#h", 3);

  
  int *arg = (int *)malloc(sizeof(*arg));
  *arg = sockfd;
  pthread_create(&child, NULL, recieve_message_thread, arg);

  while(1){
    n = read(sockfd, recvBuff, sizeof(recvBuff)-1);
    recvBuff[n] = 0;

    if(recvBuff[0]=='#' && recvBuff[1]=='e' && recvBuff[2]=='x' && recvBuff[3]=='i' && recvBuff[4]=='t'){
        break;
    }

    printf("\r%s", recvBuff);
    fflush(stdout);
  }
  


  printf("Goodbye, come again!\n");
  return 0;
}