#include <stdio.h>
#include "cJSON.h"
#include "cGPT.h"

int main(){
    ChatClient* client = createClient("hello");
    if(client == NULL){
        printf("Failed to create client \n");
    }else{
        printf("Successfully created client\n");
    }
    return 0;
}