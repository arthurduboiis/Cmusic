//
// Created by tanguy on 30/03/2022.
//
#include "common.h"
#include "tcp.h"
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include "flag_thread.h"
#include "lecteur_audio.h"
//#include <json-c/json.h>
#include <time.h>


_Bool is_playing = 0;
_Bool stopL = 0;

int thread_tcp(void* arg){
    time_t seconds;
    time_t seconds2;
    time_t diff;
    int id_temp;
    char* loop = NULL;

    char message[16];
    int s;
    struct sockaddr_in server;

    if((s = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        fprintf(stderr, "couldn't create socket");
        return -1;
    }

    char* adresse = strtok(arg, ":");
    char* port = strtok(NULL, ":");

    server.sin_addr.s_addr = inet_addr(adresse);
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(port));

    while(connect(s, (struct sockaddr*)&server, sizeof(server)) < 0){
        fprintf(stderr, "connect error");
        SDL_Delay(10000);
    }

//    message = "C/TCP/GET/musique/0";
//    if(send(s, message, strlen(message), 0) < 0){
//        return-1;
//    }
//    if(recv(s, reponse, 1000, 0) < 0){
//        fprintf(stderr, "no response");
//        return 0;
//    }

//    strtok(reponse, "/");
//    strtok(NULL, "/");
//    char* temp = strtok(NULL, "/");
//    temp += '\0';
//
//    char* json = strtok(temp, ",");

//    struct json_object* parsed_json;
//
//    parsed_json = json_tokener_parse(json);
//
//    struct json_object* nom;
//    json_object_object_get_ex(parsed_json, "nom", &nom);
//
//    int temp2 = json_object_array_length(nom);
//
//    temp2 += '\0';
//    fprintf(stderr, "on est la ?");
//    fprintf(stderr, "%d", temp2);

    for(;;){
        if(quit)
            break;
        if(rrA[0])
            id = -1;
        if(rrA[4])
            loop = "OUI";


        if(playB && !is_playing && id_temp == id){
            playB = 0;
            is_playing = 1;
            SDL_CreateThread(thread_lecteur_audio, "thread_lecteur_audio", "rtmp://localhost/live/STREAM_NAME");
            sprintf(message, "C/TCP/SEEK/%d/%ld/%s", id, diff, loop);

            if(send(s, message, strlen(message), 0) < 0){
                break;
            }

            seconds2 = time(NULL);
        }
        else if(playB && !is_playing){
            playB = 0;
            is_playing = 1;
            SDL_CreateThread(thread_lecteur_audio, "thread_lecteur_audio", "rtmp://localhost/live/STREAM_NAME");

            sprintf(message, "C/TCP/START/%d/%s/%s", id, (char*)NULL, loop);
            id_temp = id;

            if(send(s, message, strlen(message), 0) < 0){
                break;
            }
            seconds = time(NULL);

        }
        if(playB && is_playing){
            playB = 0;
            is_playing = 0;
            stopL = 1;
            seconds2 = time(NULL);
        }
        diff = seconds2 - seconds;
    }


//    if(send(s, message, strlen(message), 0) < 0){
//        puts("Send failed");
//        return 1;
//    }
//    puts("Data Send\n");
//    if(recv(s, reponse, 1000, 0) < 0){
//        puts("no response");
//        return 0;
//    }
//    strcat(reponse, "\0");
//    printf("%s\n", reponse);


    return 0;
}

