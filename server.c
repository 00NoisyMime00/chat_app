#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>


struct User{
    int id;
    char name[100];
};

pthread_t child[10000];
pthread_mutex_t lock; 
struct User connected_users[10000];
int user_count = 0;


struct thread_args{
    struct User arg_user;
};

int send_to_client(char message[], int clientfd){
    write(clientfd, message, strlen(message));
    return 0;
}

void notify_all(struct User connected_users[10000], int user_count, struct User new_user, char *action){
    int clientfd;
    char buff[15];
    char *message = (char *)malloc(strlen(new_user.name) + 3 + strlen("-joined") + 1);

    strcat(message, new_user.name);
    strcat(message, "-");
    snprintf(buff, 15, "%d %s\n", new_user.id, action);
    strcat(message, buff);
    printf("notifying all-%s\n", message);

    for(int i = 0; i < user_count; i++){
        send_to_client(message, connected_users[i].id);
    }
}

char * get_all_connected_users(struct User connected_users[10000], int user_count){
    // printf("reached inside!\n");
    char *users = (char *)malloc(10000);
    char buff[3];
    // strcpy(users, connected_users[0].id);
    // printf("final %s\n",users);
    for(int i = 0; i < user_count; i++){
        strcat(users, connected_users[i].name);
        strcat(users, "-");
        snprintf(buff, 3, "%d", connected_users[i].id);
        // printf("buff is %s\n", buff);
        strcat(users, buff);
        strcat(users, "\n");
    }
    return users;
}

void remove_user(int clientfd, struct User connected_users[], int user_count){
    printf("trying to remove %d from total %d users online!\n", clientfd, user_count);
    if(user_count == 1 || user_count == 10000){
        struct User n;
        connected_users[0] = n;
        // printf("%s", connected_users[0].name);
        return;
    }
    
    for(int i = 0; i < user_count; i++){
        printf("found %d need %d\n", connected_users[i].id, clientfd);
        if(connected_users[i].id == clientfd){
            printf("Found and removing! %d\n", connected_users[i].id);
            if(i == user_count - 1){
                struct User n;
                connected_users[i] = n;
                return;
            }
            for(int j = i; j < user_count; j++){
                connected_users[j] = connected_users[j + 1];
            }
            return;
        }
    }
}

int recieve_from_client(int clientfd){
    char recvBuff[1000];
    int n = 1;
    while(n != 0){
        n = read(clientfd, recvBuff, sizeof(recvBuff)-1);
        recvBuff[n] = 0;

        if(fputs(recvBuff, stdout) == EOF){
            printf("\n Error : Fputs error");
      }
    }

    return 0;
}

void *handle_client(void *_args){
    // close(listenfd);
    struct thread_args *args = (struct thread_args *)_args;
    int connfd = args->arg_user.id;

    printf("starting thread for %d\n", connfd);

    recieve_from_client(connfd);
    printf("closing %d\n", connfd);

// lock starts
    pthread_mutex_lock(&lock);

    user_count --;
    remove_user(connfd, connected_users, user_count+1);
    notify_all(connected_users, user_count, args->arg_user, "Left");

    pthread_mutex_unlock(&lock);
// unlocked!

    close(connfd);
    free(args);
    // exit(0);
}
 
int main(void){
    int listenfd = 0,connfd = 0;
  
    struct sockaddr_in serv_addr;
 
    char sendBuff[1025];
    char readBuff[1025]; 
    char name[100]; 
    int numrv;  
    pid_t pid;
 
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    printf("socket retrieve success\n");

    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));
      
    serv_addr.sin_family = AF_INET;    
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    serv_addr.sin_port = htons(5000);    
    
    bind(listenfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr));
    
    if(listen(listenfd, 10) == -1){
        printf("Failed to listen\n");
        return -1;
    }    

    if (pthread_mutex_init(&lock, NULL) != 0){ 
        printf("\n mutex init has failed\n"); 
        return 1; 
    }  
    
    while(1){      
        connfd = accept(listenfd, (struct sockaddr*)NULL ,NULL); // accept awaiting request

        int n = read(connfd, name, sizeof(name)-1);
        name[n] = 0;

        struct User new_user;
        new_user.id = connfd;
        strcpy(new_user.name, name);

        notify_all(connected_users, user_count, new_user, "Joined");

        user_count ++;
        connected_users[user_count - 1] = new_user;

        strcpy(sendBuff, "You are connected to server!\nAll Users:\n");
        strcat(sendBuff, get_all_connected_users(connected_users, user_count));
        write(connfd, sendBuff, strlen(sendBuff));
        
        printf("connected %s-%d\n", name, connfd);

        struct thread_args *args = malloc(sizeof (struct thread_args));
        args->arg_user = new_user;
        printf("did thread start?\n");
        pthread_create(&child[user_count - 1], NULL, handle_client, args);

        // pid = fork();
        // if (pid < 0)
        //     perror("ERROR on fork");
            
        // if (pid == 0)  {
            
        // }


        char *u = get_all_connected_users(connected_users, user_count);
        printf("here-->\n%s", u);
        printf("---end---\n");
           
        sleep(1);
    } 
    
    return 0;
}