#include <stdio.h>
//#include <winsock2.h>
#include <arpa/inet.h>
//#include <unistd.h>
//#include <stdlib.h>
#include <string.h>
#include <malloc.h>
//Creation d'un socket, connexion et communication en TCP

int main(int argc, char* argv[]) {

//    WSADATA wsa;
    int s;
    struct sockaddr_in server;
    char* message;
    char reponse[100];
    int recv_size;

//    printf("\nInitialising Winsock ...");
//    if(WSAStartup(MAKEWORD(2,2),&wsa) != 0){
//        printf("Failed. Errod Code : %d", WSAGetLastError());
//        return 1;
//    }

    printf("Initialised\n");

    if((s = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("Could not create socket ");
        return 1;
    }

    printf("socket created\n");

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);


    //Connect to remote server
    if(connect(s, (struct sockaddr*)&server, sizeof(server)) < 0){
        puts("connect error");
        return 1;
    }
    puts("Connected");

    //Send some data

    message = "C/TCP/START/1";
    if(send(s, message, strlen(message), 0) < 0){
        puts("Send failed");
        return 1;
    }
    puts("Data Send\n");
    if(recv(s, reponse, 100, 0) < 0){
	puts("no response");
	return 0;
    }

printf("%s", reponse);
fflush(stdout);
//    closesocket(s);
//    WSACleanup();

    return 0;
}
