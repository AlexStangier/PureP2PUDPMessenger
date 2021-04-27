//
// Created by Alexander Stangier on 19.04.21.
//

#ifndef VS_LAB1_LAB02_H
#define VS_LAB1_LAB02_H

#define MAXSIZE 512             //Max size for messages
#define NAMESIZE 24             //Max size for usernames

enum methods {
    JOIN,                       //method to join a existing peer group
    JOIN_RESPONSE,              //response after successfully connecting to a peer group contains a list of existing peers
    SEND,                       //method to send a message to other peers
    EXIT                        //method to shut down current instance and to disconnect from peers
};

struct messagepdu {
    enum methods method;        //sets the appropriate method type from methods
    short group;                //set the desired group
    char name[NAMESIZE];        //contains the username
    short ttl;                  //controls the repetation of the message
    char message[MAXSIZE];      //contains the actual message depending on method type message[0] contains the counter
};

#endif //VS_LAB1_LAB02_H
