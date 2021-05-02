#include <stdlib.h>
#include <stdio.h>

typedef struct adress {
    char ipadr[15];
    char port[10];
    short index;
    short group;
    struct adress *next;
} adress;

static short listIndex = 0;

void printList();

void removeDuplicateElements(adress *head);

void push(adress **head, adress *sadr);

void push(adress **head, adress *sadr) {
    adress *new_node;
    new_node = malloc(sizeof(adress));

    strcpy(new_node->ipadr, sadr->ipadr);
    strcpy(new_node->port, sadr->port);
    new_node->index = listIndex++;
    new_node->group = sadr->group;
    new_node->next = *head;
    *head = new_node;

    removeDuplicateElements(*head);
    //printf("\n");
    //printList(head);
}

void printList(adress **head) {
    adress *current = *head;
    while (strcmp(current->ipadr, "anchor") != 0) {
        printf("index: %hd\n", current->index);
        printf("ip: %s\n", current->ipadr);
        printf("port: %s\n", current->port);
        current = current->next;
    }
}

void pop(adress **head) {
    adress *next_node = NULL;
    next_node = (*head)->next;

    if (head != NULL) {
        //free(*head);
        *head = next_node;
    }
    //printList(head);
}

void removeEntry(adress **head, adress val) {
    adress *temp_node, *ptrval, *current = NULL;
    current = *head;
    ptrval = &val;

    if (ptrval != NULL) {
        if (strcmp((*head)->ipadr, ptrval->ipadr) == 0 && strcmp((*head)->port, ptrval->port) == 0) {
            pop(head);
        }
        while (strcmp(current->next->ipadr, ptrval->ipadr) != 0 && strcmp(current->next->port, ptrval->ipadr) != 0) {
            if (current->next == NULL) {
                return;
            }
            current = current->next;
        }

        temp_node = current->next;
        current->next = temp_node->next;
        if (temp_node != NULL)
            free(temp_node);
    }
}

char *getList(adress **head) {
    adress *current = *head;
    char *string = malloc(MAXSIZE * sizeof(char));
    while (strcmp(current->ipadr, "anchor") != 0) {
        //concat result to string
        char ipv4[15];
        char port[10];
        strcpy(ipv4, current->ipadr);
        strcpy(port, current->port);

        strcat(string, ipv4);
        strcat(string, ":");
        strcat(string, port);
        strcat(string, ",");

        current = current->next;
    }

    return string;
}

/* Function to remove duplicates from a sorted list */
void removeDuplicateElements(adress *head) {
    adress *ptr1, *ptr2, *duplicate = NULL;
    ptr1 = head;

    while (ptr1 != NULL && strcmp(ptr1->next->ipadr, "anchor") != 0 && strcmp(ptr1->ipadr, "anchor") != 0) {
        ptr2 = ptr1;

        /* Compare the current element with rest of the elements */
        while (strcmp(ptr2->next->ipadr, "anchor") != 0) {
            if (strcmp(ptr1->ipadr, ptr2->next->ipadr) == 0 && strcmp(ptr1->port, ptr2->next->port) == 0) {
                duplicate = ptr2->next;
                ptr2->next = ptr2->next->next;
                free(duplicate);
            } else
                ptr2 = ptr2->next;
        }
        ptr1 = ptr1->next;
    }
}



