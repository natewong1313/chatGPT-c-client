#include <stdio.h>
#include <stdlib.h>
#include "cGPT.h"
#include <stdio.h>
#include <curl/curl.h>

int main(){
    ChatMessage messages[1] = {{
        "user", "tell me a dad joke"
    }};
    ChatParams params = {
        .apikey=getenv("API_KEY"), 
        .model="gpt-3.5-turbo", 
        .messages=messages, 
        .messages_size=sizeof(messages)
    };
    ChatResponse *chatResp = create_chat_completion(&params);
    printf("Response: id: %s, object: %s, created %d\n", chatResp->id, chatResp->object, chatResp->created);
    return 0;
}