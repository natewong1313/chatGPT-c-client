#include <stdio.h>
#include <stdlib.h>
#include "cGPT.h"

int main(){
    ChatClient* client = createChatClient(getenv("API_KEY"));
    if(client == NULL){
        printf("Failed to create client \n");
    }else{
        printf("Successfully created client\n");
    }
    createChatCompletion(client);
    return 0;
}