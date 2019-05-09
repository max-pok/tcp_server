//
// Created by Max Pokidaylo on 2019-05-01.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "DataTree.h"

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    /* Create the listener socket as TCP socket */
    int sd = socket( AF_INET, SOCK_STREAM, 0 );
    /* here, the sd is a socket descriptor (part of the fd table) */

    if (sd == -1) {
        perror("socket() failed.");
        return EXIT_FAILURE;
    }

    /* socket structures */
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);  /* allow any IP address to connect */

    /* htons() is host-to-network short for data marshalling */
    /* Internet is big endian; Intel is little endian */
    server.sin_port = htons(PORT);
    int opt_val = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));

    int len = sizeof( server );

    if (bind(sd, (struct sockaddr *)&server, (socklen_t) len) == -1) {
        perror("bind() failed.");
        return EXIT_FAILURE;
    }


    /* identify this port as a listener port */
    if (listen( sd, 5 ) == -1) {
        perror("listen() failed.");
        return EXIT_FAILURE;
    }
    printf("server is listening on %d\n", PORT);

    struct sockaddr_in client;
    int fromlen = sizeof(client);

    int n;
    pid_t pid;
    char buffer[BUFFER_SIZE];
    bzero(buffer,BUFFER_SIZE);
    for (int i = 0 ; i < 100 ; i++) {
        int newsd = accept(sd, (struct sockaddr *) &client, (socklen_t *) &fromlen);
        char buf[255];
        FILE *readME;
        readME = fopen("TCPMainManu.txt", "r");
        if (readME) {
            while (fgets(buf, 255, readME) != NULL) {
                strcat(buf,"\0");
                send(newsd, buf, strlen(buf) ,0);
            }
        }
        else printf("file error\n");
        pid = fork();

        if ( pid == -1 ) {
            perror( "fork() failed" );
            return EXIT_FAILURE;
        }

        if ( pid > 0 ) {
            /* parent simply closes the new client socket
                because the child process will handle that connection */
            close( newsd );
        }
        else /* pid == 0 */ {
            send(newsd, "\n\n", sizeof("\n\n"), 0);
            printf("%s connected\n", inet_ntoa(client.sin_addr));
            do {
                send(newsd, "Enter your command: ", sizeof("Enter your command: "), 0);
                n = (int) recv(newsd, buffer, BUFFER_SIZE, 0);
                buffer[n] = '\0';
                if (n == -1) {
                    perror("recv() failed");
                    return EXIT_FAILURE;
                } else if (strcmp(buffer, "exit\n") == 0) {
                    i--;
                    printf("%s disconnected\n", inet_ntoa(client.sin_addr));
                    send(newsd, "GoodBye!\n", strlen("GoodBye!\n"), 0);
                } else {
                    option2(newsd, buffer);
                    //system("./check.sh -p matrix.py m6.in");
                }
            } while (strcmp(buffer, "exit\n") != 0);
            close(newsd);
        }
    }

    return EXIT_SUCCESS;
}