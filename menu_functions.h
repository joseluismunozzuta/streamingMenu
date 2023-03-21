#include <curl/curl.h>
#include "cJSON.h"
#include <unistd.h>
#include <signal.h>

#define ROWS 4
#define COLS 6
#define IMAGE_WIDTH 250
#define IMAGE_HEIGHT 130
#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

//Declare functions
cJSON *readJson(const char *url);

// Auxiliar structure
struct MemoryStruct
{
    char *memory;
    size_t size;
};

//Auxiliar functions

//Handle the json request
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL)
    {
        printf("Not enough memory (realloc returned NULL)\n");
        return 0;
    }

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

//Send a request to an URL to obtain the json object to return
cJSON *readJson(const char *url)
{
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;
    chunk.memory = NULL;
    chunk.size = 0;
    cJSON *json;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            printf("Error retrieving data: %s\n", curl_easy_strerror(res));
        }
        else
        {
            json = cJSON_Parse(chunk.memory);
            if (json == NULL)
            {
                const char *error_ptr = cJSON_GetErrorPtr();
                if (error_ptr != NULL)
                {
                    printf("Error: %s\n", error_ptr);
                }
                return NULL;
            }
        }
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    if (chunk.memory)
    {
        free(chunk.memory);
    }

    return json;
}