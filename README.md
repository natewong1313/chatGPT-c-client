# ChatGPT Client
This project is a client library for OpenAI's new ChatGPT api. You can view the documentation for the api [here](https://platform.openai.com/docs/api-reference/chat/create).

# Important
To use this project, you'll need an OpenAI api key. They offer a free trial so you can go ahead and play around with this project. Read more about [getting your api key](https://openai.com/blog/openai-api)

# Install
This library aims to have a small footprint so you can get this up and running in no time!

## Step 1. Install cURL/libcurl
This library uses [libcurl](https://curl.se/libcurl/) to make API requests. 
#### Linux
```
sudo apt update && sudo apt upgrade
sudo apt-install libcurl4-openssl-dev
```
#### Windows
```
choco install curl
```
#### Mac
libcurl should come preinstalled, you can verify by running
```
curl --version
```

## Step 2. Clone repository
```
git clone https://github.com/natewong1313/chatGPT-c-client.git
```
And that's it!

# Usage
To use this library, you'll need to include the cGPT and cJSON header and source code files in your project. Those are located in the include and src folders in this repository.
```
├── include/
│   ├── cGPT.h
│   └── cJSON.h
├── src/
│   ├── cGPT.c
│   └── cJSON.c
└── ...
```
> When building your project, make sure to include the libcurl library `-lcurl` in your makefile.

## Basic example
```c
#include "cGPT.h"
#include <stdlib.h>

int main(){
    ChatMessage messages[1] = {{
        "user", "Tell me a dad joke!"
    }}; 
    ChatParams params = {
        .apikey=getenv("API_KEY"), 
        .model="gpt-3.5-turbo",
        .messages=messages, 
        .messages_size=1
    };
    ChatCompletion *chat_completion = create_chat_completion(&params);
    if(chat_completion != NULL){
        printf("%s\n", chat_completion->choices[0].message.content);
    }
    return 0;
}
```

## ChatCompletion structure
The create_chat_completion function returns a pointer to a ChatCompletion struct. Here's what it looks like
```c
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
```
You can view the original json response of the OpenAI api [here](https://platform.openai.com/docs/api-reference/chat/create)

# License
This project is [MIT licensed](LICENSE)
