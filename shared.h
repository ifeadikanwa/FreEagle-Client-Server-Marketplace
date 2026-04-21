#ifndef SHARED_H
#define SHARED_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

// default server settings
#define SERVER_PORT 8000
#define SERVER_IP "127.0.0.1"

// max users that can have notifier sockets
#define MAX_NOTIFIERS 50

// item categories
typedef enum
{
    supplies,
    books,
    furniture,
    misc
} Category;

// message sent from client to server
typedef struct
{
    enum
    {
        createNotifier,
        login,
        offer,
        request,
        getList,
        removeRequest,
        removeOffer,
        logout
    } messageType;

    unsigned int userID; 
    Category category;
    unsigned int offerID;
    char message[100];
    char location[20];
} toSA;

// message sent from server to client
typedef struct
{
    enum
    {
        ackLogin,
        ackOffer,
        ackRequest,
        ackList,
        ackListCount,
        ackRemoveRequest,
        ackRemoveOffer,
        notification,
        ackLogout
    } messageType;

    unsigned int userID;
    Category category;
    unsigned int offerID;
    char message[100];
    char location[20];
} toCA;

// notifier socket info for logged in users
typedef struct
{
    int socket;
    unsigned int userID;
    int isLoggedIn;
} NotifierClient;

// shared notifier array stored in server file
extern NotifierClient activeNotifiers[MAX_NOTIFIERS];

// helper functions
const char *convertCategoryToString(Category category);
void HandleClient(int clntSock);

// offer functions
void SaveOfferToFileDB(toSA clientRequest);
unsigned int GetNextOfferID();
void GetOfferList(int clntSock, Category requestedCategory);
int RemoveOfferFromFileDB(unsigned int offerID, unsigned int userID);

// request functions
void SaveRequestToFileDB(unsigned int userID, Category category);
int RemoveRequestFromFileDB(unsigned int userID, Category category);

// notification functions
void SendNotificationsToUsers(toSA clientRequest);
void SendInitialNotifications(int clntSock, unsigned int userID);
void CleanUpNotifierArray();
void RemoveNotifierByUserID(unsigned int userID);

#endif