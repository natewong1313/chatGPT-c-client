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
static ChatCompletion *parse_response(char *response){
    cJSON *parsed_json = cJSON_Parse(response);
    if (parsed_json == NULL) {
        // free mem
        cJSON_Delete(parsed_json);
        return NULL;
    }

    cJSON *choices = cJSON_GetObjectItem(parsed_json, "choices");
    int num_choices = cJSON_GetArraySize(choices);
    ChatCompletion *chat_completion = malloc(sizeof(ChatCompletion));
    // allocate memory to the pointer
    chat_completion->choices = malloc(sizeof(Choice) * num_choices);
    // update num_choices
    chat_completion->num_choices = num_choices;
    // parse id
    char *id_str = cJSON_GetStringValue(cJSON_GetObjectItem(parsed_json, "id"));
    chat_completion->id = malloc(strlen(id_str + sizeof("")));
    strcpy(chat_completion->id, id_str);
    // parse object
    char *object_str = cJSON_GetStringValue(cJSON_GetObjectItem(parsed_json, "object"));
    chat_completion->object = malloc(strlen(object_str + sizeof("")));
    strcpy(chat_completion->object, object_str);
    // parse created
    chat_completion->created = cJSON_GetNumberValue(cJSON_GetObjectItem(parsed_json, "created"));
    // parse model
    char *model_str = cJSON_GetStringValue(cJSON_GetObjectItem(parsed_json, "model"));
    chat_completion->model = malloc(strlen(model_str + sizeof("")));
    strcpy(chat_completion->model, model_str);
    // parse usage
    cJSON *usage = cJSON_GetObjectItem(parsed_json, "usage");
    chat_completion->usage.prompt_tokens = cJSON_GetNumberValue(cJSON_GetObjectItem(usage, "prompt_tokens"));
    chat_completion->usage.completion_tokens = cJSON_GetNumberValue(cJSON_GetObjectItem(usage, "completion_tokens"));
    chat_completion->usage.total_tokens = cJSON_GetNumberValue(cJSON_GetObjectItem(usage, "total_tokens"));

    for (int i = 0; i < num_choices; i++) {
        // get the first item from the choices array "choices":[{"message":{"role":"","content":""}, "finish_reason":"stop","index":0}]
        cJSON *choice_item = cJSON_GetArrayItem(choices, i);
        // get the message obj "message":{"role":"","content":""}
        cJSON *message_item = cJSON_GetObjectItem(choice_item, "message");
        // parse role
        char *role_str =  cJSON_GetStringValue(cJSON_GetObjectItem(message_item, "role"));
        chat_completion->choices[i].message.role  = malloc(strlen(role_str + sizeof("")));
        strcpy(chat_completion->choices[i].message.role, role_str);
        // parse content
        char *content_str =  cJSON_GetStringValue(cJSON_GetObjectItem(message_item, "content"));
        chat_completion->choices[i].message.content  = malloc(strlen(content_str + sizeof("")));
        strcpy(chat_completion->choices[i].message.content, content_str);

        // parse finish_reason
        char *finish_reason_str = cJSON_GetStringValue(cJSON_GetObjectItem(choice_item, "finish_reason"));
        chat_completion->choices[i].finish_reason  = malloc(strlen(finish_reason_str + sizeof("")));
        strcpy(chat_completion->choices[i].finish_reason, finish_reason_str);
        // parse index
        chat_completion->choices[i].index = cJSON_GetNumberValue(cJSON_GetObjectItem(choice_item, "index"));
    }
    // free mem
    cJSON_Delete(parsed_json);
    return chat_completion;
}
// parse an error
static char *parse_error(char *response){
    cJSON *parsed_json = cJSON_Parse(response);
    if (parsed_json == NULL) {
        // free mem
        cJSON_Delete(parsed_json);
        return NULL;
    }
    cJSON *error_obj = cJSON_GetObjectItem(parsed_json, "error");
    // get error message, create a pointer, set it to error message
    char *error_msg = cJSON_GetStringValue(cJSON_GetObjectItem(error_obj, "message"));
    char *error = malloc(strlen(error_msg) + sizeof(""));
    strcpy(error, error_msg);

    cJSON_Delete(parsed_json);
    return error;
}
// Create a completion for specified chat message
ChatCompletion *create_chat_completion(ChatParams *params){
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
        // response pointer
        ChatCompletion *chat_completion;
        // status code of request 
        int res_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res_code);
        int success = 0;
        if(res_code == 200){
            chat_completion = parse_response(chunk.response);
            success = 1;
            if(chat_completion == NULL){
                fprintf(stderr, "Unknown error parsing response body\n");
                success = 0;
            }
        }else{
            char *error_msg = parse_error(chunk.response);
            fprintf(stderr, "Got status code %d, %s\n", res_code, error_msg);
        }
        // always cleanup so no memory leaks!
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers_list);
        curl_global_cleanup();
        free(chunk.response);
        free(req_body);
        if(success){
            return chat_completion;
        }else{
            return NULL;
        }
    }
    curl_global_cleanup();
    return NULL;
}