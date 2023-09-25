#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/select.h>

int main(int argc, char *argv[])
{
    int socket_client;
    struct sockaddr_in server_address;
    char buffer[1024];
    char pseudo[32];
    char message[1024];

    // Vérification des arguments
    if (argc < 3)
    {
        fprintf(stderr, "Usage : %s adresse_ip port\n", argv[0]);
        return 1;
    }



    socket_client = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_client < 0)
    {
        perror("Erreur lors de la création de la socket");
        return 1;
    }


    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(argv[1]);
    server_address.sin_port = htons(atoi(argv[2]));

        // Connexion au serveur
        if (connect(socket_client, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
        {
            perror("Erreur lors de la connexion au serveur");
            return 1;
        }
    while(1){
        // Demande du pseudo au lancement
        printf("Entrez le pseudo : ");
        fgets(pseudo, 32, stdin);

        // Envoi du pseudo au serveur
        write(socket_client, pseudo, strlen(pseudo));


        bzero(buffer, strlen(buffer));
        //récéption de la vérification du pseudo
        int n = read(socket_client, buffer, 2);
        if (n < 0)
        {
            perror("Erreur lors de la réception des données");
            return 1;
        }

        //pseudo libre
        if(buffer[0] == '1'){
            printf("client ajouté\n");
            break;
        }
        //pseudo déjà utilisé
        else if(buffer[0] == '0'){
            printf("Pseudo Already Used\n");
        }
        //autre
        else{
            printf("Problème avec le serveur");
        }
        
    }


    while (1)
    {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(socket_client, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        int activity = select(socket_client + 1, &read_fds, NULL, NULL, NULL);

        if (activity < 0)
        {
            perror("Erreur lors de l'utilisation de select()");
            return 1;
        }

        // Vérifier si l'entrée standard est prête à être lue
        if (FD_ISSET(STDIN_FILENO, &read_fds))
        {
            bzero(buffer, strlen(buffer));
            fgets(buffer, 1024, stdin);
            write(socket_client, buffer, strlen(buffer));
        }

        // Vérifier si la socket client est prête à être lue
        if (FD_ISSET(socket_client, &read_fds))
        {
            // Réception des messages du serveur
            bzero(buffer, strlen(buffer));
            int n = read(socket_client, buffer, 1024);
            if (n < 0)
            {
                perror("Erreur lors de la réception des données");
                return 1;
            }

            time_t rawtime;
            struct tm *timeinfo;
            time(&rawtime);
            timeinfo = localtime(&rawtime);
            strftime(message, 1024, "[%H:%M:%S] ", timeinfo);
            strcat(message, buffer);
            printf("%s\n", message);
        }
    }
    return 0;
}
