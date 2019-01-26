#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

const int port = 3000;

int n;
int fd_list[1024];

void add_fd(int fd){
    fd_list[n++] = fd;
}

void rm_fd(int fd){
    int found = 0;
    for (int i=0; i<n; ++i){
        if (found){
            fd_list[i-1] = fd;
        }else{
            if (fd_list[i] == fd) found=1;
        }
    }
    --n;
    close(fd);
}

void send_msg(char *msg, int len, int fd){
    for (int i=0; i<n; ++i){
        if (fd_list[i] != fd){
            write(fd_list[i], msg, len);
        }
    }
}

void *handle_connection(void* fda){
    int fd = *(int*)fda;
    add_fd(fd);
    char msg[1024];
    char name[64];
    write(fd, "Enter name:\n", 12);
    int len = read(fd, name, 64);

    name[len-2] = 0;
    strcpy(msg, name);
    strcat(msg, " says: ");
    len = strlen(msg);

    while(1){
        int len2;
        if ((len2 = read(fd, msg+len, 1022-len) ) > 0){
            send_msg(msg, len+len2, fd);
        }else{
            rm_fd(fd);
            return NULL;
        }
    }
}

int main(int argc, char const *argv[])
{
    //signal(SIGPIPE, SIG_IGN); //ignore sigipe

    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(-1);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(-1);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( port );

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(-1);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(-1);
    }


    while (1){
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
        {
            perror("accept");
            exit(-1);
        }
        pthread_t tid;
        pthread_create(&tid, NULL, &handle_connection, (void *)(&new_socket));
    }
    return 0;
}
