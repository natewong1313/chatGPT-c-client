#include "cGPT.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "cJSON.h"

// callback from the api request
// https://curl.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
static size_t write_memory_callback(void *data, size_t size, size_t nmemb, void *clientp){
    size_t realsize = size  *nmemb;
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
// take in parameters and return json request body as string
static char *build_req_body(char *model, ChatMessage *messages, int messages_size){
    // create object: {}
    cJSON *body_obj = cJSON_CreateObject();
    cJSON *model_str = cJSON_CreateString(model);
    // {model: ""}
    cJSON_AddItemToObject(body_obj, "model", model_str);
    cJSON *messages_arr = cJSON_CreateArray();
    // {model: "", messages: []}
    cJSON_AddItemToObject(body_obj, "messages", messages_arr);
    cJSON *message_obj = NULL;
    cJSON *role_str = NULL;
    cJSON *content_str = NULL;
    for(int i = 0; i < messages_size; i++){
        message_obj = cJSON_CreateObject();
        cJSON_AddItemToArray(messages_arr, message_obj);
        // {model: "", messages: [{role: ""}]}
        role_str = cJSON_CreateString(messages[i].role);
        cJSON_AddItemToObject(message_obj, "role", role_str);
        // {model: "", messages: [{role: "", content: ""}]}
        content_str = cJSON_CreateString(messages[i].content);
        cJSON_AddItemToObject(message_obj, "content", content_str);
    }
    return cJSON_Print(body_obj);
}
// validate value for key in json response
static int str_key_valid(cJSON *obj){
    return cJSON_IsString(obj) && (obj->valuestring != NULL);
}
static int int_key_valid(cJSON *obj){
    return cJSON_IsNumber(obj);
}
// parse response json body
static ChatResponse *parse_response(char *response){
    // parse response using cJSON
    cJSON *parsed_json = cJSON_Parse(response);
    if (parsed_json != NULL) {
        // create pointer to struct
        ChatResponse *chat_response_ptr = malloc(sizeof(ChatResponse));
        // start parsing away
        cJSON *id_str = cJSON_GetObjectItemCaseSensitive(parsed_json, "id");
        if (str_key_valid(id_str)) {
            chat_response_ptr->id = malloc(strlen(id_str->valuestring));
            strcpy(chat_response_ptr->id, id_str->valuestring);
        }

        cJSON *object_str = cJSON_GetObjectItemCaseSensitive(parsed_json, "object");
        if (str_key_valid(object_str)) {
            chat_response_ptr->object = malloc(strlen(object_str->valuestring));
            strcpy(chat_response_ptr->object, object_str->valuestring);
        }

        cJSON *created_int = cJSON_GetObjectItemCaseSensitive(parsed_json, "created");
        if (int_key_valid(created_int)) {
            chat_response_ptr->created = created_int->valueint;
        }

        cJSON *model_str = cJSON_GetObjectItemCaseSensitive(parsed_json, "model");
        if (str_key_valid(model_str)) {
            chat_response_ptr->model = malloc(strlen(model_str->valuestring));
            strcpy(chat_response_ptr->model, model_str->valuestring);
        }

        // cJSON *choices_arr = cJSON_GetObjectItemCaseSensitive(parsed_json, "choices");
        // cJSON *choice_obj;
        // Choice choices[cJSON_GetArraySize(choices_arr)];
        // printf("Array size: %d\n", cJSON_GetArraySize(choices_arr));
        // // Choice choicesArr[cJSON_GetArraySize(choices_arr)];
        // int i = 1;
        // choices[0].finish_reason = "Finish reason 1";
        // choices[0].index = 100;
        // // cJSON_ArrayForEach(choice_obj, choices_arr){
        // //     cJSON *finish_reason = cJSON_GetObjectItemCaseSensitive(choice_obj, "finish_reason");
        // //     //finish_reason->valuestring
        // //     choices[i].finish_reason = "munch";
        // //     // char *json = cJSON_Print(choice_obj);
        // //     // printf("%s", json);
        // //     // cJSON_free(json);
        // //     // cJSON *index = cJSON_GetObjectItemCaseSensitive(choice, "index");
        // //     // choices_arr[i].index = index->valueint;
        // //     // cJSON *finish_reason = cJSON_GetObjectItemCaseSensitive(choice, "finish_reason");


        // //     // messageObj->content = malloc(strlen(content->valuestring)+1);
        // //     // choicesArr[i].finish_reason = finish_reason->valuestring;
            
        // //     // Message *messageObj = malloc(sizeof(Message));
        // //     // cJSON *message = cJSON_GetObjectItemCaseSensitive(choice, "message");
        // //     // cJSON *role = cJSON_GetObjectItemCaseSensitive(message, "role");
        // //     // messageObj->role = malloc(strlen(role->valuestring)+1);
        // //     // strcpy(messageObj->role, role->valuestring);
        // //     // cJSON *content = cJSON_GetObjectItemCaseSensitive(message, "content");
        // //     // messageObj->content = malloc(strlen(content->valuestring)+1);
        // //     // strcpy(messageObj->content, content->valuestring);
        // //     // choicesArr[i].message = messageObj;
        // //     i++;
        // // }
        // // chat_response_ptr->choices = 
        // chat_response_ptr->choices = choices;
        // chat_response_ptr->choices_size = i;
        cJSON_Delete(parsed_json);
        return chat_response_ptr;
    }
    cJSON_Delete(parsed_json);
    return 0;
}
// Create a completion for specified chat message
ChatResponse *create_chat_completion(ChatParams *params){
    CURL *curl;
    CURLcode res;
    // apparently needed for windows?
    curl_global_init(CURL_GLOBAL_ALL);
    // initialize libcurl
    curl = curl_easy_init();
    if (curl) {
        // list of headers
        struct curl_slist *headers_list = NULL;
        // add content type to headers since we are sending a json body
        headers_list = curl_slist_append(headers_list, "Content-Type: application/json");
        // add api key to end of auth header string
        char auth_header[100] = "Authorization: Bearer ";
        strcat(auth_header, params->apikey);
        // add auth header
        headers_list = curl_slist_append(headers_list, auth_header);
        // update our headers with the headers list
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers_list);
        // set request URL
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/chat/completions");
        // make sure we send an https request
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        // set request body
        char *req_body = build_req_body(params->model, params->messages, params->messages_size);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req_body);
        // set size of request body
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(req_body));
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
        curl_slist_free_all(headers_list);
        curl_global_cleanup();
        free(chunk.response);
        free(req_body);

        return chatResp;
    }
    curl_global_cleanup();
    return 0;
}