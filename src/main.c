#include <stdio.h>
#include <stdlib.h>
#include "cGPT.h"
#include <stdio.h>
#include <curl/curl.h>

int main(){
    create_chat_completion(getenv("API_KEY"), "gpt-3.5-turbo");
    return 0;
}