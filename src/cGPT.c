#include "cGPT.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "cJSON.h"

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
// https://curl.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
static size_t write_memory_callback(void *data, size_t size, size_t nmemb, void *clientp){
    size_t realsize = size * nmemb;
    ResponseMemory *mem = (ResponseMemory *)clientp;
    char *ptr = realloc(mem->response, mem->size + realsize + 1);
    if(ptr == NULL){
        fprintf(stderr, "Response too large, out of memory.\n");
        return 0;
    }
    
    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), data, realsize);
    mem->size += realsize;
    mem->response[mem->size] = 0;
    return realsize;
}

static char* build_req_body(char *model){
    char buffer[100];
    snprintf(buffer, sizeof buffer, "{\"model\": \"%s\", \"messages\": [{\"role\": \"user\", \"content\": \"Hello!\"}]}", model);
    return strdup(buffer);
}
static int jsonStrKeyFound(cJSON* obj){
    return cJSON_IsString(obj) && (obj->valuestring != NULL);
}
static ChatResponse *parse_response(char *response){
    // parse response using cJSON
    cJSON* json = cJSON_Parse(response);
    if (json != NULL) {
        // create pointer to struct
        ChatResponse *chatResponsePtr = malloc(sizeof(ChatResponse));
        // start parsing away
        cJSON* idObj = cJSON_GetObjectItemCaseSensitive(json, "id");
        if (jsonStrKeyFound(idObj)) {
            chatResponsePtr->id = malloc(strlen(idObj->valuestring));
            strcpy(chatResponsePtr->id, idObj->valuestring);
        }

        cJSON* objectObj = cJSON_GetObjectItemCaseSensitive(json, "object");
        if (jsonStrKeyFound(objectObj)) {
            chatResponsePtr->object = malloc(strlen(objectObj->valuestring));
            strcpy(chatResponsePtr->object, objectObj->valuestring);
        }

        cJSON_Delete(json);
        return chatResponsePtr;
    }
    return 0;
}
// Create a completion for specified chat message
ChatResponse *create_chat_completion(char *apiKey, char *model){    
    CURL *curl;
    CURLcode res;
    // apparently needed for windows?
    curl_global_init(CURL_GLOBAL_ALL);
    // initialize libcurl
    curl = curl_easy_init();
    if (curl) {
        // list of headers
        struct curl_slist *headersList = NULL;
        // add content type to headers since we are sending a json body
        headersList = curl_slist_append(headersList, "Content-Type: application/json");
        // add api key to end of auth header string
        char authHeader[100] = "Authorization: Bearer ";
        strcat(authHeader, apiKey);
        // add auth header
        headersList = curl_slist_append(headersList, authHeader);
        // update our headers with the headers list
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headersList);
        // set request URL
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/chat/completions");
        // make sure we send an https request
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        // set request body
        char *reqBody = build_req_body(model);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, reqBody);
        // set size of request body
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(reqBody));
        // handle our callback for writing the response body to memory
        // first allocate memory for the response body
        ResponseMemory chunk = {.response = malloc(1), .size = 0};
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_memory_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        // send the http request
        res = curl_easy_perform(curl);
        // analyze response code
        ChatResponse *chatResp;
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            chatResp = parse_response(chunk.response);
        }
        // always cleanup so no memory leaks!
        curl_easy_cleanup(curl);
        curl_slist_free_all(headersList);
        curl_global_cleanup();
        free(chunk.response);
        free(reqBody);
        
        return chatResp;
    }
    curl_global_cleanup();
    return 0;
}