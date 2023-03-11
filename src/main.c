#include <stdio.h>
#include <stdlib.h>
#include "cGPT.h"
#include <stdio.h>
#include <curl/curl.h>

int main(){
    ChatMessage messages[1] = {{
        "user", "tell me a dad joke"
    }};
    ChatResponse *chatResp = create_chat_completion(getenv("API_KEY"), "gpt-3.5-turbo", messages, 1);
    printf("Response: id: %s, object: %s, created %d\n", chatResp->id, chatResp->object, chatResp->created);
    return 0;
}