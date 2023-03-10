#include <stdio.h>
#include <stdlib.h>
#include "cGPT.h"
#include <stdio.h>
#include <curl/curl.h>

int main(){
    ChatResponse *chatResp = create_chat_completion(getenv("API_KEY"), "gpt-3.5-turbo");
    printf("Response: id: %s, object: %s\n", chatResp->id, chatResp->object);
    return 0;
}