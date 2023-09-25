#include <stdio.h>
#include <string.h> //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>    //close
#include <arpa/inet.h> //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD SET, FD ISSET, FD ZERO macros

#define TRUE 1
#define FALSE 0
#define PORT 8000

typedef struct list_client
{
    char *pseudo;
    int socket;
    struct list_client *next;
} list_client;

list_client *head = NULL;

void add_client(int socket, char *pseudo)
{
    list_client *new_client = (list_client *)malloc(sizeof(list_client));
    new_client->socket = socket;
    new_client->pseudo = pseudo;
    new_client->next = head;
    head = new_client;
}

list_client *remove_client(list_client *head, int socket)
{
    list_client *current = head;
    list_client *previous = NULL;


    while (current != NULL)
    {
        // if the socket is found
        if (current->socket == socket)
        {

            if (previous == NULL)
            {
                head = current->next;
            }
            else
            {
                previous->next = current->next;
            }
            free(current);
            break;
        }
        previous = current;
        current = current->next;
    }
    return head;
}

void replace_char(char* buffer, char old, char new) {
    int i;
    for (i = 0; buffer[i] != '\0'; i++) {
        if (buffer[i] == old) {
            buffer[i] = new;
        }
    }
}

int verif_pseudo(char* new_pseudo)//renvoie 0 si le pseudo existe déjà sinon 1
{
    list_client *current = head;


    while (current != NULL)
    {
        if(strcmp(current->pseudo, new_pseudo)==0){
             return 0;
        }
        current = current->next;
    }
    return 1;
}


// void replaceChar(char* buffer, char old, char new){
//     for(int i = 0; i<strlen(buffer); i++){
//         if(buffer[i] == old){
//             buffer[i] == new;
//         }
//     }
    
//     return;
// }

int main(int argc, char *argv[])
{
    int opt = TRUE;
    int master_socket, addrlen, new_socket, activity, valread, sd;
    int max_sd;
    struct sockaddr_in address;
    char buffer[1025];
    char sender[1025];
    fd_set readfds;

    if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }


    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }


    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(PORT);


    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", PORT);

 
    if (listen(master_socket, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }


    addrlen = sizeof(address);
    puts("Waiting for connections ...");

    while (TRUE)
    {

        FD_ZERO(&readfds);
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        list_client *current = head;
        while (current != NULL)
        {
            sd = current->socket;
            if (sd > max_sd)
                max_sd = sd;
            FD_SET(sd, &readfds);
            current = current->next;
        }

        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR))
        {
            printf("select error");
        }


        if (FD_ISSET(master_socket, &readfds))
        {
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }


            printf("New connection , socket fd is %d , ip is : %s , port : %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));


            //store pseudo
            char *pseudo = (char *)malloc(sizeof(char) * 1025);
            bzero(buffer, strlen(buffer));
            bzero(pseudo, strlen(pseudo));
            valread = read(new_socket, buffer, 1024);
            strcpy(pseudo, buffer);

            //if not exist
            if(verif_pseudo(pseudo)==1){
                add_client(new_socket, pseudo);
                write(new_socket, "1", 2);
                printf("Nouvelle connection au nom de : %s\n", pseudo);
            }else{
                add_client(new_socket, "\0");
                printf("pas le bon pseudo\n");
                write(new_socket, "0", 2);
            }

        }


        list_client *currentb = head;
        while (currentb != NULL)
        {
            bzero(buffer, strlen(buffer));

            sd = currentb->socket;
            strcpy(sender, currentb->pseudo);

            

            if (FD_ISSET(sd, &readfds))
            {
                //read message or check for closing connection
                if ((valread = read(sd, buffer, 1024)) == 0)
                {

                    getpeername(sd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
                    printf("Client disconnected , ip %s , port %d \n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));


                    close(sd);

                    head = remove_client(head, sd);
                }


                else
                {
                    char* this_pseudo = (char *)malloc(sizeof(char) * 1025);
                    strcpy(this_pseudo, buffer);
                    if((currentb->pseudo)[0] == '\0'){//if client have no pseudo
                        if(verif_pseudo(this_pseudo)==1){
                            currentb->pseudo = this_pseudo;
                            write(new_socket, "1", 2);
                            printf("Nouvelle connection apres pseudo fail au nom de : %s\n", this_pseudo);
                        }else{
                            write(new_socket, "0", 2);
                            bzero(this_pseudo, 1025);
                            free(this_pseudo);

                        }
                    
                    }else if(buffer[0] == '/'){//if client write command
                        char* token = (char *)malloc(sizeof(char) * 1025);
                        strcpy(token, strtok(buffer, " "));

                        if(strcmp(token, "/nickname") == 0){//change pseudo
                            char * newpseudo = strtok(NULL, " ");
                            if(verif_pseudo(newpseudo) == 1){
                                currentb->pseudo = newpseudo;
                                write(sd, "Changement de pseudo effectué\n", 32);
                            }else{
                                write(sd, "Pseudo Already Used\n", 21);
                            }
                        }


                    }else{
                        //send message for all clients
                        buffer[valread] = '\0';
                        list_client *current_write = head;
                        strcat(sender, " >>> ");
                        strcat(sender, buffer);
                        replace_char(sender, '\n', ' ');
                        while (current_write != NULL)
                        {
                            sd = current_write->socket;

                            write(sd, sender, 1024);
                            current_write = current_write->next;
                        }


                    }
                    bzero(sender, 1025);

                }
            }
            currentb = currentb->next;
        }
    }

    return 0;
}