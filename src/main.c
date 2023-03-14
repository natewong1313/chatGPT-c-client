#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cGPT.h"
#include <stdio.h>
#include <curl/curl.h>

int main(){
    ChatMessage messages[1] = {{
        "user", "Who won the world series in 2020? Who won in 2019?"
    }};
    ChatParams params = {
        .apikey=getenv("API_KEY"), 
        .model="gpt-3.5-turbo",
        .messages=messages, 
        .messages_size=1
    };
    ChatCompletion *chat_completion = create_chat_completion(&params);
    if(chat_completion != NULL){
        printf("Response\n");
        printf("id: %s\nobject: %s\ncreated: %d\nmodel: %s\n", chat_completion->id, chat_completion->object, chat_completion->created, chat_completion->model);
        printf("usage: {\n");
        printf("    prompt_tokens: %d\n", chat_completion->usage.prompt_tokens);
        printf("    completion_tokens: %d\n", chat_completion->usage.completion_tokens);
        printf("    total_tokens: %d\n", chat_completion->usage.total_tokens);
        printf("}\n");

        for (int i = 0; i < chat_completion->num_choices; i++) {
            printf("Choice %d:\n", i+1);
            printf("Message role: %s\n", chat_completion->choices[i].message.role);
            printf("Message content: %s\n", chat_completion->choices[i].message.content);
            printf("Finish reason: %s\n", chat_completion->choices[i].finish_reason);
            printf("Index: %d\n", chat_completion->choices[i].index);
            printf("\n");
        }
        free(chat_completion->choices);
        free(chat_completion);
    }
    return 0;
}