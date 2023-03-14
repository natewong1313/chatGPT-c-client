#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cGPT.h"
#include <stdio.h>
#include <curl/curl.h>

int main(){
    ChatMessage messages[1] = {{
        "user", "Who won the world series in 2020?"
    }};
    ChatParams params = {
        .apikey=getenv("API_KEY"), 
        .model="gpt-3.5-turbo",
        .messages=messages, 
        .messages_size=1
    };
    ChatResponse *chat_resp = create_chat_completion(&params);
    printf("Response\n");
    printf("id: %s\nobject: %s\ncreated: %d\nmodel: %s\n", chat_resp->id, chat_resp->object, chat_resp->created, chat_resp->model);
    // Choice *choices = chat_resp->choices;
    // printf("choices: [\n");
    // for(int i = 0; i < chat_resp->choices_size; i++){
    //     printf("{\n");
    //     printf("    index: %d\n", chat_resp->choices[i].index);
    //     // printf("    finish_reason: %s\n", chat_resp->choices[i].finish_reason);
    //     // chat_resp->choices[i];
       
    //     // printf("%s\n", choices[i].message->role);
    //     printf("}\n");
    // }
    return 0;
}