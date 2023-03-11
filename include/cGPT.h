#include <curl/curl.h>
#include "cJSON.h"

typedef struct {
    char* apiKey;
} ChatClient;

typedef struct {
    char* role;
    char* content;
} ChatMessage;

typedef struct {
    char *response;
    size_t size;
} ResponseMemory;

typedef struct {
    char* id;
    char* object;
    int created;
    char* model;
} ChatResponse;

static size_t write_memory_callback(void *contents, size_t size, size_t nmemb, void *userp);
static char *build_req_body(char *model, ChatMessage *messages, int messagesSize);
static int jsonStrKeyFound(cJSON* obj);
static int jsonIntKeyFound(cJSON* obj);
static ChatResponse *parse_response(char *response);
ChatResponse *create_chat_completion(char *apiKey, char *model, ChatMessage *messages, int messagesSize);