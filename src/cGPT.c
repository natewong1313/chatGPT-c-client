#include "cGPT.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

ChatClient* createClient(char* apiKey){
    if ((apiKey == NULL) || (apiKey[0] == '\0')) {
        fprintf(stderr, "Invalid API key parameter. Must be of type \"string\" with length > 0\n");
        return NULL;
    }
    ChatClient* chatClient = malloc(sizeof(ChatClient));
    chatClient->apiKey = malloc(sizeof(apiKey));
    strcpy(chatClient->apiKey, apiKey);
    return chatClient;
}