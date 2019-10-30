#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include <ctype.h>


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
    char *m = (char *)malloc(strlen(message) + strlen("You: ")+5);
    strcpy(m, message);
    strcat(m, "\nyou: ");
    // printf("here? %s\n", m);
    write(clientfd, m, strlen(m));
    return 0;
}

void group_chat(char message[], struct User connected_users[], int user_count, int sender){
    for(int i = 0; i < user_count; i++){
        if(connected_users[i].id != sender)
            send_to_client(message, connected_users[i].id);
    }
}

void notify_all(struct User connected_users[10000], int user_count, struct User new_user, char *action){
    int clientfd;
    char buff[15];
    char *message = (char *)malloc(strlen("-14 Joined\n")+strlen(new_user.name));

    strcpy(message, new_user.name);
    strcat(message, "-");
    snprintf(buff, 15, "%d %s\n", new_user.id, action);
    strcat(message, buff);
    printf("notifying all-%s\n", message);

    for(int i = 0; i < user_count; i++){
        send_to_client(message, connected_users[i].id);
    }
}

char * get_all_connected_users(struct User connected_users[10000], int user_count){
    
    char *users = (char *)malloc(10000);
    char buff[3];
    for(int i = 0; i < user_count; i++){
        strcat(users, connected_users[i].name);
        strcat(users, "-");
        snprintf(buff, 3, "%d", connected_users[i].id);
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

int check_validity_of_fd(int fd, struct User connected_users[], int user_count){
    for(int i = 0; i < user_count; i++){
        if(fd == connected_users[i].id)
            return 1;
    }
    return 0;
}

int recieve_from_client(struct User user, int *chat_option){
    char recvBuff[1000];
    int n = 1;
    int clientfd = user.id;

    char name[100];
    strcpy(name, user.name);

    while(n != 0){
        n = read(clientfd, recvBuff, sizeof(recvBuff)-1);
        
        if(n == 0){
            return 0;
        }

        recvBuff[n] = 0;
        char buff[3];


        if(n == 3 && recvBuff[0] == '#'){
            if(recvBuff[1] == '0'){
                *chat_option = 0;
                send_to_client("\n Switched to Group chat!\n", clientfd);
            }
            else if(recvBuff[1] == 'a'){
                char * message = (char *)malloc(23+sizeof(get_all_connected_users(connected_users, user_count)));
                strcpy(message, "All Active Users:\n");
                strcat(message, get_all_connected_users(connected_users, user_count));
                send_to_client(message, clientfd);
            }
            else if(recvBuff[1] == 'h'){
                char * message = "------HELP-------\n#h - help\n#a - Show all connected users\n#c - Get the person you're currently messaging\n#i - Get Your ID\n#client_id - message the respective client(0 for group)\n";
                send_to_client(message, clientfd);
            }
            else if(recvBuff[1] == 'i'){
                char *message = (char *)malloc(strlen("Your ID is: 55\n") + 1);
                sprintf(message, "Your ID is: %d\n", clientfd);
                printf("the message is: %s", message);
                send_to_client(message, clientfd);
                
            }
            else if(recvBuff[1] == 'c'){
                char *message = (char *)malloc(sizeof("You are messaging: 55")+1);
                sprintf(message, "You are messaging: %d\n", *chat_option);
                // write(clientfd, message, strlen(message));
                send_to_client(message, clientfd);
            }
            else if(isdigit(recvBuff[1]) && atoi(&recvBuff[1]) != clientfd){
                (*chat_option) = atoi(&recvBuff[1]);
                printf("chat option is %d\n", *chat_option);
                if(!check_validity_of_fd(*chat_option, connected_users, user_count) || *chat_option == clientfd){
                    send_to_client("ERROR!, either the ID is Invalid or Disconnected, Switching to group chat!\n", clientfd);
                    *chat_option = 0;
                }
                else{
                    char *message = (char *)malloc(sizeof("\nSending message to client with ID 5\n"));
                    sprintf(message, "\nSending message to client with ID %d\n", *chat_option);
                    send_to_client(message, clientfd);
                }
            }
            else{
                send_to_client("INVALID COMMAND!!\n", clientfd);
            }
        }

        else if((n == 4) && recvBuff[0] == '#'){
            if(isdigit(recvBuff[1]) && isdigit(recvBuff[2])){
                // printf("r1: %s r2:%s\n", &recvBuff[1], &recvBuff[2]);
                (*chat_option) = atoi(&recvBuff[1]);
                printf("chat option is %d\n", *chat_option);
                if(!check_validity_of_fd(*chat_option, connected_users, user_count) || *chat_option == clientfd){
                    send_to_client("ERROR!, either the ID is Invalid or Disconnected, Switching to group chat!\n", clientfd);
                    *chat_option = 0;
                }
                else{
                    char *message = (char *)malloc(sizeof("\nSending message to client with ID 5\n"));
                    sprintf(message, "\nSending message to client with ID %d\n", *chat_option);
                    send_to_client(message, clientfd);
                }
            }
            else{
                send_to_client("INVALID COMMAND!!\n", clientfd);
            }
        }
        else{

            char *message = (char *)malloc(sizeof(name)+5+sizeof(recvBuff));
            strcpy(message, name);
            strcat(message, "-");
            snprintf(buff, 3, "%d", clientfd);
            strcat(message, buff);
            strcat(message, ":");
            
            strcat(message, recvBuff);
            printf("message: %s", message);

            printf("chat option is %d and clientfd is %d\n", *chat_option, clientfd);
            if(*chat_option == 0){
                group_chat(message, connected_users, user_count, clientfd);
                send_to_client("", clientfd);
            }
            else{
                printf("checking validity %d\n", check_validity_of_fd(*chat_option, connected_users, user_count));
                if(check_validity_of_fd(*chat_option, connected_users, user_count) && *chat_option != clientfd){
                    printf("chat option found..");
                    send_to_client(message, *chat_option);
                    send_to_client("", clientfd);
                }
                else{
                    send_to_client("Person You're trying to message maybe left, Changed to default group chat!\n", clientfd);
                    printf("changing chat option to default 0\n");
                    *chat_option = 0;
                }
            }
        }
    }

    return 0;
}

void *handle_client(void *_args){
    
    struct thread_args *args = (struct thread_args *)_args;
    int connfd = args->arg_user.id;
    int chat_option = 0;

    printf("starting thread for %d\n", connfd);

    recieve_from_client(args->arg_user, &chat_option);
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
}
 
int main(void){
    char socketname[] = "socket";
    int listenfd = 0,connfd = 0;
  
    struct sockaddr_un serv_addr;
 
    char sendBuff[1025];
    char readBuff[1025]; 
    char name[100]; 
    int numrv;  
    pid_t pid;
 
    listenfd = socket(AF_UNIX, SOCK_STREAM, 0);
    printf("socket retrieve success\n");

    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));
      
    serv_addr.sun_family = AF_UNIX;    
    strcpy(serv_addr.sun_path, socketname);
    
    // unlinking is important ref: https://troydhanson.github.io/network/Unix_domain_sockets.html
    unlink(socketname);
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
        // write(connfd, sendBuff, strlen(sendBuff));
        send_to_client(sendBuff, connfd);
        
        printf("connected %s-%d\n", name, connfd);

        struct thread_args *args = (struct thread_args *)malloc(sizeof (struct thread_args));
        args->arg_user = new_user;
        pthread_create(&child[user_count - 1], NULL, handle_client, args);


        // char *u = get_all_connected_users(connected_users, user_count);
        // printf("here-->\n%s", u);
        // printf("---end---\n");
           
        sleep(1);
    } 
    
    return 0;
}