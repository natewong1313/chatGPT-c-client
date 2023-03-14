#include <curl/curl.h>
#include "cJSON.h"

typedef struct {
    char *role;
    char *content;
} ChatMessage;

typedef struct {
    char *apikey;
    char model[50];
    ChatMessage *messages;
    int messages_size;
} ChatParams;

typedef struct {
    char *response;
    size_t size;
} ResponseMemory;

typedef struct {
    int prompt_tokens;
    int completion_tokens;
    int total_tokens;
} Usage;

typedef struct {
    char *role;
    char *content;
} Message;

typedef struct {
    Message message;
    char *finish_reason;
    int index;
} Choice;

typedef struct {
    char *id;
    char *object;
    int created;
    char *model;
    Usage usage;
    Choice *choices;
    int num_choices;
} ChatCompletion;

static size_t write_memory_callback(void *contents, size_t size, size_t nmemb, void *userp);
static char *build_req_body(char *model, ChatMessage *messages, int messages_size);
static int str_key_valid(cJSON *obj);
static int int_key_valid(cJSON *obj);
static ChatCompletion *parse_response(char *response);
static char *parse_error(char *response);
ChatCompletion *create_chat_completion(ChatParams *params);