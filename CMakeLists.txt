cmake_minimum_required(VERSION 3.25.2)

project(CGPTClient
    VERSION 1.0
    DESCRIPTION "A client library for Chat GPT"
    LANGUAGES C)

add_library(CGPTClient STATIC 
    src/cJSON.c
    src/cGPT.c
)

target_include_directories(CGPTClient
    PUBLIC 
        ${PROJECT_SOURCE_DIR}/include
)

add_executable(cgpt
    src/main.c)

target_link_libraries(cgpt
    PRIVATE 
        CGPTClient
)