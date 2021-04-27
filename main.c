#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/select.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "lab02.h"
#include "linkedList.h"


//buffer: stdin, name : stdin, argv: classparams
void createAndSendMessage(char *buffer, short type, char *name, short ttl, adress **head);

/*
 * Loops through given list of peers and adds them to the local peerlist
 */
void addPeersToList(char message[512], adress **head);

/*
 * Send the referenced message via udp socket to all peers
 */
void sendMessageToAll(struct messagepdu *msg, adress **head);

/*
 * Send message to specified address
 */
void sendMessageToAddress(char *ipv4, char *port, struct messagepdu *msg);

/*
 * Reply to join method with current peerlist
 */
void sendJoinReply(char *ipv4, char *port, adress **head);

/*
 * Send message to second program parameter
 */
void sendMessageToProgramParameter(struct messagepdu *msgJoin, char **argv);

/*
 * Pointer to peer list
 */
adress *head;

/*
 * Addresses have to inherit the following structures in order to be used by the application:
 * {a.b.c.address} {xxxxx} where the first parameter is the ipv4 address and the second parameter
 * is defining the desired port.
 * The first pair will be used for your own Peer.
 */
int main(int argc, char **argv) {
    int ssocket, retval;
    char buffer[MAXSIZE];
    struct messagepdu receiveBuffer;
    struct sockaddr_in si_me;
    char *nameptr = malloc(NAMESIZE * sizeof(char));
    int *group = malloc(sizeof(short));

    head = malloc(sizeof(adress));
    if (head == NULL) {
        return 1;
    }

    adress *sadr0 = malloc(sizeof(adress));
    strcpy(sadr0->port, "1337");
    strcpy(sadr0->ipadr, "anchor");

    //push anchor element
    push(&head, sadr0);

    adress *sadr1 = malloc(sizeof(adress));
    strcpy(sadr1->port, argv[2]);
    strcpy(sadr1->ipadr, argv[1]);

    //push own address
    push(&head, sadr1);

    //check parameter count
    if (argc < 5) {
        printf("invalid amount of arguments\n");
        return EXIT_FAILURE;
    }

    //create udp socket
    if ((ssocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("udp socket binding failed\n");
        return EXIT_FAILURE;
    }

    //get username
    printf("Please enter a name to join the chat:\n");
    scanf("%s", nameptr);

    //get group
    printf("Please enter a group number:\n");
    scanf("%i", group);
    printf("%s joined the chat!\n", nameptr);

    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(atoi(argv[2]));
    si_me.sin_addr.s_addr = inet_addr(argv[1]);

    //bind socket
    if (bind(ssocket, (struct sockaddr *) &si_me, sizeof(si_me)) == -1) {
        perror("bind failed\n");
        return EXIT_FAILURE;
    }

    struct messagepdu *msgJoin = malloc(sizeof(struct messagepdu));

    //create join message and send 2 program parameter
    char ipadrJoin[21];
    strcat(ipadrJoin, argv[1]);
    strcat(ipadrJoin, ":");
    strcat(ipadrJoin, argv[2]);
    strcpy(msgJoin->message, ipadrJoin);

    msgJoin->ttl = 1;
    strcpy(msgJoin->name, nameptr);
    msgJoin->method = JOIN;
    msgJoin->group = *group;

    sendMessageToProgramParameter(msgJoin, argv);

    //set filedescriptors
    fd_set s_rd;
    while (1) {
        //set fds for stdin and udp
        FD_SET(ssocket, &s_rd); //UDP
        FD_SET(fileno(stdin), &s_rd);   //STDIN
        retval = select(ssocket + 1, &s_rd, NULL, NULL, NULL);

        //print out result from select
        if (retval == -1) {
            perror("select failed\n");
            return EXIT_FAILURE;
        } else if (retval == 0) {
            perror("timeout occured\n");
        } else {
            //Message received

            //messages received from udp
            if (FD_ISSET(ssocket, &s_rd)) {
                //udp package
                struct sockaddr_in si_peer;
                memset((char *) &si_peer, 0, sizeof(si_peer));
                recvfrom(ssocket, &receiveBuffer, sizeof(struct messagepdu), 0, (struct sockaddr *) &si_peer, 0);

                //handle message types
                //handle join
                if (receiveBuffer.method == JOIN) {
                    //copy message
                    char *string;
                    char *ptr;
                    string = strdup(receiveBuffer.message);
                    strtok_r(string, ":", &ptr);

                    //reply with peerlist
                    sendJoinReply(string, ptr, &head);

                    // add new peer to list
                    adress *newPeer = malloc(sizeof(adress));

                    //add new address to list
                    strcpy(newPeer->ipadr, string);
                    strcpy(newPeer->port, ptr);

                    //add to list of groups match
                    push(&head, newPeer);

                    //echo new peer to other peers and set ttl to 0 in order to avoid zombie messages
                    //only relay message if current peer is first responder
                    if (receiveBuffer.ttl != 0) {
                        receiveBuffer.ttl = 0;
                        sendMessageToAll(&receiveBuffer, &head);
                    }

                    printf("%s ... has joined the conversation.\n", receiveBuffer.name);
                }
                //handle join with peerlist
                if (receiveBuffer.method == JOIN_RESPONSE) {
                    addPeersToList(receiveBuffer.message, &head);
                }
                //handle send
                if (receiveBuffer.method == SEND) {
                    printf("%s ... was sent by %s.\n", receiveBuffer.message, receiveBuffer.name);
                }
                //handle exit
                if (receiveBuffer.method == EXIT) {

                    char *string;
                    char *ptr;
                    string = strdup(receiveBuffer.message);
                    strtok_r(string, ":", &ptr);

                    adress *peerToRemove = malloc(sizeof(adress));

                    //remove address from list
                    strcpy(peerToRemove->ipadr, string);
                    strcpy(peerToRemove->port, ptr);

                    printf("port to be removed: %s\n", ptr);

                    removeEntry(&head, *peerToRemove);

                    printf("%s ... has left the conversation.\n", receiveBuffer.name);
                }
            }

            //monitor keyboard input
            if (FD_ISSET(fileno(stdin), &s_rd)) {
                //Console input
                fgets(buffer, sizeof(buffer), stdin);
                if (strlen(buffer) > 1) {
                    printf("%s ... was read from stdin.\n", buffer);
                    if (strncmp(buffer, "!EXIT", 5) == 0) {
                        //EXIT conversation

                        //send exit
                        char msgToExit[21];
                        strcat(msgToExit, argv[1]);
                        strcat(msgToExit, ":");
                        strcat(msgToExit, argv[2]);
                        createAndSendMessage((char *) &msgToExit, EXIT, nameptr, 0, &head);

                        break;
                    } else {
                        //SEND message
                        createAndSendMessage(buffer, SEND, nameptr, 0, &head);
                    }
                }
            }
        }
    }

    printf("Left conversation.");

    close(ssocket);

    return EXIT_SUCCESS;
}

void sendMessageToAll(struct messagepdu *msg, adress **head) {
    int csocket;
    struct sockaddr_in servaddr;

    if ((csocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed\n");
        exit(EXIT_FAILURE);
    }

    adress *current = *head;

    //loop peer list until the anchor is reached and send messages
    //avoid sending message to yourself by excluding index 1
    while (strcmp(current->ipadr, "anchor") != 0 && current->index != 1) {
        // Fill server information
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = inet_addr(current->ipadr);
        servaddr.sin_port = htons(atoi(current->port));

        sendto(csocket, msg, sizeof(struct messagepdu),
               0, (const struct sockaddr *) &servaddr,
               sizeof(servaddr));

        current = current->next;
    }

    close(csocket);
}

void sendMessageToAddress(char *ipv4, char *port, struct messagepdu *msg) {
    int csocket;

    if ((csocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in si_peer;

    memset((char *) &si_peer, 0, sizeof(si_peer));
    si_peer.sin_family = AF_INET;
    si_peer.sin_port = htons(atoi(port));
    si_peer.sin_addr.s_addr = inet_addr(ipv4);

    sendto(csocket, msg, sizeof(struct messagepdu),
           0, (const struct sockaddr *) &si_peer,
           sizeof(si_peer));

    close(csocket);
}

void createAndSendMessage(char *buffer, short type, char *name, short ttl, adress **head) {

    //create messagepdu
    struct messagepdu message;
    message.method = type;
    message.ttl = ttl;
    strncpy(message.name, name, sizeof(message.name));
    strncpy(message.message, buffer, strlen(buffer));

    //send Message
    sendMessageToAll(&message, head);
}

void sendMessageToProgramParameter(struct messagepdu *msgJoin, char **argv) {
    int csocket;
    struct sockaddr_in si_peer;

    if ((csocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed\n");
        exit(EXIT_FAILURE);
    }

    memset((char *) &si_peer, 0, sizeof(si_peer));
    si_peer.sin_family = AF_INET;
    si_peer.sin_port = htons(atoi(argv[4]));
    si_peer.sin_addr.s_addr = inet_addr(argv[3]);

    sendto(csocket, msgJoin, sizeof(struct messagepdu),
           0, (const struct sockaddr *) &si_peer,
           sizeof(si_peer));

    close(csocket);
}

void sendJoinReply(char *ipv4, char *port, adress **head) {
    struct messagepdu message;
    message.method = JOIN_RESPONSE;
    message.ttl = 0;
    char *ptr;
    ptr = getList(head);
    strcpy(message.message, ptr);

    sendMessageToAddress(ipv4, port, &message);
}

void addPeersToList(char *message, adress **head) {
    //iterate list with delimiter
    char *ptr;
    int i = 0;
    ptr = strtok(message, ",:");
    adress *padr = malloc(sizeof(adress));

    while (ptr != NULL) {
        i++;
        //add port or ip to list
        if (i % 2) {
            strcpy(padr->ipadr, ptr);
        } else {
            strcpy(padr->port, ptr);
            push(head, padr);
        }

        ptr = strtok(NULL, ",:");
    }
}

