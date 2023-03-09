#include "cGPT.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>

ChatClient* createChatClient(char* apiKey){
    if ((apiKey == NULL) || (apiKey[0] == '\0')) {
        fprintf(stderr, "Invalid API key parameter. Must be of type \"string\" with length > 0\n");
        return NULL;
    }
    ChatClient* chatClient = malloc(sizeof(ChatClient));
    chatClient->apiKey = malloc(sizeof(apiKey));
    strcpy(chatClient->apiKey, apiKey);
    return chatClient;
}

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    size_t realsize = size * nmemb; 
    RequestMemory *req = (RequestMemory *) userdata;

    printf("receive chunk of %zu bytes\n", realsize);

    while (req->buflen < req->len + realsize + 1)
    {
        req->buffer = realloc(req->buffer, req->buflen + CHUNK_SIZE);
        req->buflen += CHUNK_SIZE;
    }
    memcpy(&req->buffer[req->len], ptr, realsize);
    req->len += realsize;
    req->buffer[req->len] = 0;

    return realsize;
}

void createChatCompletion(ChatClient* client){    
    CURL *curl = malloc(sizeof(CURL));
    CURLcode res;
    // curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    curl_easy_cleanup(curl);
    // if(curl){
    //     struct curl_slist *headersList = NULL;
    //     headersList = curl_slist_append(headersList, sprintf("Authorization: Bearer %s", client->apiKey));
    //     headersList = curl_slist_append(headersList, "Content-Type: application/json");
    //     curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headersList);
    //     // set request url
    //     curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/chat/completions");
    //     curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
    //     // set json body
    //     curl_easy_setopt(curl, CURLOPT_POSTFIELDS, 
    //         "{\"model\" : \"gpt-3.5-turbo\", \"messages\": [{\"role\": \"user\", \"content\": \"Hello!\"}]}"
    //     );
    //     // chunk for storing the response in memory to read later
    //     // RequestMemory req = {.buffer = NULL, .len = 0, .buflen = 0};

    //     // req.buffer = malloc(CHUNK_SIZE);
    //     // req.buflen = CHUNK_SIZE;

    //     // curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    //     // curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&req);
    //     // send request
    //     res = curl_easy_perform(curl);
    //     // printf("Result = %u\n",res);

    //     // printf("Total received bytes: %zu\n", responseChunk.size);
    //     // if(res != CURLE_OK){
    //     //     fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    //     // }else{
    //     //     printf("Received data:/n%s\n", responseChunk.memory);
    //     // }
    //     /* always cleanup */
    //     curl_easy_cleanup(curl);
    
    //     /* free the custom headers */
    //     curl_slist_free_all(headersList);

    //     // free(req.buffer);

    // }
    // curl_global_cleanup();
}