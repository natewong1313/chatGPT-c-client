# ChatGPT Client
OpenAI's ChatGPT client library written in C

# Install
This library aims to have a small footprint so you can get this up and running in no time!

## Step 1. Install cURL/libcurl
This library uses [libcurl](https://curl.se/libcurl/) to make API requests. 
#### Linux
```
$ sudo apt update && sudo apt upgrade
$ sudo apt install curl
```
#### Windows
```
$ choco install curl
```
#### Mac
libcurl should come preinstalled, you can verify by running
```
$ curl --version
```

## Step 2. Clone repository
```
git clone https://github.com/natewong1313/ChatGPT-C-Client.git
```