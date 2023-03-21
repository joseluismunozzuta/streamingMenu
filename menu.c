#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <curl/curl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "cJSON.h"
#include <unistd.h>
#include <signal.h>
#include "menu_functions.h"

//Util strings
const char *dirRefIdPath = "refid_dir";
const char *ref1url = "https://cd-static.bamgrid.com/dp-117731241344/sets/";
const char *ref2url = ".json";
const char *slash = "/";
const char *text_refId = "refId";
const char *homeURL = "https://cd-static.bamgrid.com/dp-117731241344/home.json";
const char loadingtext[50] = "Loading... please wait";
const char *jpg_format = ".jpg";

// Variables for selecting images and display different rows
int selected_image = 0;
int ofsets_rows[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
int ofset_col = 0;
int ofset_row_names = 0;

// Variable for controlling the loop
int quit = 0;

// Setting colors
SDL_Color black = {0, 0, 0, 0};
SDL_Color white = {255, 255, 255, 255};

// Create a render
SDL_Renderer *renderer;

// Variable for loading a font
TTF_Font *font;

// Char array for holding the refId's
char refIdsArray[9][100];

// Char array for holding the masterId of each image
char masterIdsArray[9][15][100];

// Char array for holding each collection name
char refIdCollectionNameArray[9][50];

// SDL surface double dimensioned for showing the images
SDL_Surface *image_surfaces[ROWS][COLS];

// SDL surface for showing the name of each row
SDL_Surface *rowname_surfaces[ROWS];

// JSON object obtained from the API
cJSON *homeJSON;

/*Declare the functions created*/
char *buildUrlwithRefId(char *refId);
char *buildRefIDPath(int indexRefId);
char *buildImageFullPath(char *refIdPath, int row, int colum);
void clearScreen();
void createrefId_dir();
void event_Handler();
void renderMenu();
void loadImagesToShow();
void renderDisneyLoading();
void obtainInfoHomeJson(cJSON *homeJson);
void manageInfoFromRefIdJson(cJSON *json, int index_masterId_arr);
void download_image(const char *master_id, const char *url, const char *path);
void updateSurfaces(int row, int ofset_vertical, int col);
void updateScrollUp(int row, int col);
void updateScrollDown(int row, int col);

// Main function
int main(int argc, char *argv[])
{

    // Initialize SDL and SDL_ttf
    SDL_Init(SDL_INIT_VIDEO);

    // Creating a window and renderer
    SDL_Window *window = SDL_CreateWindow("Disney Plus Menu", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Clear the screen
    clearScreen();

    // Initializing SDL_ttf
    if (TTF_Init() != 0)
    {
        printf("Error,%s \n", TTF_GetError());
    }

    // Load a font
    font = TTF_OpenFont("assets/fonts/PTC55F.ttf", 24);
    if (!font)
    {
        printf("Error,%s \n", TTF_GetError());
    }

    // Render the Disney loading at the beginning
    renderDisneyLoading();

    // Obtain the home.json
    homeJSON = readJson(homeURL);

    // Read home.json in order to populate an array with the URL's to obtain the images.
    obtainInfoHomeJson(homeJSON);

    // Create directory for refIds directory if it doesn't exist
    createrefId_dir();

    // As we have 9 refIds in the home.json, then we read 9 other JSON's.
    for (int x = 0; x < 9; x++)
    {
        printf("The JSON to read has the refId: %s\n", refIdsArray[x]);
        // Build the URL to send the request
        char *refIdURL = buildUrlwithRefId(refIdsArray[x]);
        // Obtain the info of the json of the current refId
        cJSON *refIdJSON = readJson(refIdURL);
        // Manage the info obtained
        manageInfoFromRefIdJson(refIdJSON, x);
    }

    // At this point, every image is already downloaded.

    // This function load the images that are saved as files in the directory refid_dir
    loadImagesToShow();

    // We clear the screen
    clearScreen();

    /*We render the menu for the first time*/
    renderMenu();

    // General Loop controlled with 'quit' variable
    while (!quit)
    {
        // This function waits for an event (key input) and handle it.
        // It contains logic about how to change the images shown.
        event_Handler();

        // Clearing the screen
        clearScreen();

        // Re render the menu
        renderMenu();
    }

    // End of the program
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

// Build the path of an image (refid_dir + master_id + .jpg)
char *buildImageFullPath(char *refIdPath, int row, int colum)
{
    // Build the final path
    //  1. Add the refIdPath
    static char fullpath[200] = "";
    memset(fullpath, 0, sizeof(fullpath));
    strncat(fullpath, refIdPath, strlen(refIdPath));
    strncat(fullpath, slash, strlen(slash));
    // 2. Concat refId path with image name
    strncat(fullpath, masterIdsArray[row][colum], strlen(masterIdsArray[row][colum]));
    strncat(fullpath, jpg_format, strlen(jpg_format));
    return fullpath;
}

//Build the URL for obtaining a json object with a ref_id
char *buildUrlwithRefId(char *refId)
{

    static char URLtoRead[100] = "";
    memset(URLtoRead, 0, sizeof(URLtoRead));
    strncat(URLtoRead, ref1url, strlen(ref1url));
    strncat(URLtoRead, refId, strlen(refId));
    strncat(URLtoRead, ref2url, strlen(ref2url));

    return URLtoRead;
}

//Build the path with a ref_id to create a dir into the refid_dir
char *buildRefIDPath(int indexRefId)
{

    static char path[100] = "";
    memset(path, 0, sizeof(path));
    strncat(path, dirRefIdPath, strlen(dirRefIdPath));
    strncat(path, slash, strlen(slash));
    strncat(path, refIdsArray[indexRefId], strlen(refIdsArray[indexRefId]));

    return path;
}

// Clears the screen
void clearScreen()
{
    // Clear the renderer
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
}

// Create the refid_dir
void createrefId_dir()
{
    char refpath[100] = {};
    strncpy(refpath, dirRefIdPath, strlen(dirRefIdPath));
    struct stat st = {0};
    if (stat(refpath, &st) == -1)
    {
        printf("Creating directory in: %s\n", refpath);
        mkdir(refpath, 0777);
    }
}

// Download the images from the collection. Save the images with the MASTERID name.
void download_image(const char *master_id, const char *url, const char *path)
{

    // Build image name with format
    char imageName[100] = {};
    strncat(imageName, master_id, strlen(master_id));
    strncat(imageName, jpg_format, strlen(jpg_format));

    // Construct full file path
    char file_path[FILENAME_MAX] = {};
    strncat(file_path, path, strlen(path));
    strncat(file_path, slash, strlen(slash));
    strncat(file_path, imageName, strlen(imageName));

    // Download and save the file
    CURL *curl_handle;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl_handle = curl_easy_init();
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);

    // Check if the file exists
    if (access(file_path, F_OK) != -1)
    {
        printf("The file %s exists. Skip download\n", file_path);
    }
    else
    {
        FILE *fp = fopen(file_path, "wb");
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl_handle);
        fclose(fp);
        if (res != CURLE_OK)
        {
            fprintf(stderr, "Error in request: %s\n", curl_easy_strerror(res));
            curl_easy_cleanup(curl_handle);
        }
    }

    curl_easy_cleanup(curl_handle);
}

//Handle the events of arrow keys input
void event_Handler()
{

    SDL_Event event;
    // Wait for an event
    while (SDL_WaitEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            quit = 1;
            break;
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym)
            {
            case SDLK_ESCAPE:
                quit = 1;
                break;
            case SDLK_LEFT:

                if (selected_image % COLS > 0)
                {
                    selected_image--;
                }
                else
                {
                    if (ofsets_rows[ofset_col + selected_image / COLS] > 0)
                    {
                        ofsets_rows[selected_image / COLS + ofset_col]--;
                        updateSurfaces(selected_image / COLS, ofset_col, (selected_image % COLS) + COLS - 1 + ofsets_rows[selected_image / COLS + ofset_col]);
                    }
                }
                break;
            case SDLK_RIGHT:

                if (selected_image % COLS < COLS - 1)
                {
                    selected_image++;
                }
                else
                {
                    if (ofsets_rows[selected_image / COLS + ofset_col] < 15 - COLS)
                    {
                        ofsets_rows[selected_image / COLS + ofset_col]++;
                        updateSurfaces(selected_image / COLS, ofset_col, selected_image % COLS + ofsets_rows[selected_image / COLS + ofset_col]);
                    }
                }
                break;
            case SDLK_UP:

                if (selected_image >= COLS)
                {
                    selected_image -= COLS;
                }
                else
                {
                    if (ofset_col > 0)
                    {
                        ofset_col--;
                        updateScrollUp(selected_image / COLS + ofset_col, ofsets_rows[selected_image / COLS + ofset_col]);
                    }
                }
                break;
            case SDLK_DOWN:

                if (selected_image < (ROWS - 1) * COLS)
                {
                    selected_image += COLS;
                }
                else
                {
                    if (ofset_col < 9 - ROWS)
                    {
                        ofset_col++;
                        updateScrollDown(selected_image / COLS + ofset_col, ofsets_rows[selected_image / COLS + ofset_col]);
                    }
                }
                break;
            }

        default:
            break;
        }
        break;
    }
}

// Using the images path, loads the images in the SDL surface
void loadImagesToShow()
{
    for (int i = 0; i < ROWS; i++)
    {
        // Construct the refID path
        char *refIdPATH = buildRefIDPath(i);

        for (int j = 0; j < COLS; j++)
        {
            // Build image full path
            char *finalpath = buildImageFullPath(refIdPATH, i, j);
            // Loading the images to show in the menu
            image_surfaces[i][j] = IMG_Load(finalpath);
        }
    }
}

// Reads the json info and handle it
void manageInfoFromRefIdJson(cJSON *json, int index_masterId_arr)
{

    cJSON *data = cJSON_GetObjectItemCaseSensitive(json, "data");
    if (data != NULL)
    {

        // Create directory if it doesn't exist
        char *dir_path = buildRefIDPath(index_masterId_arr);
        printf("dir: %s\n", dir_path);
        struct stat st = {0};
        if (stat(dir_path, &st) == -1)
        {
            mkdir(dir_path, 0777);
        }

        // Obtain data
        cJSON *items = cJSON_GetObjectItemCaseSensitive(data->child, "items");

        // Obtain name of the collection
        cJSON *text_data = cJSON_GetObjectItemCaseSensitive(data->child, "text");
        char *collection_name = text_data->child->child->child->child->child->valuestring;

        // Store the name of the collection
        strncpy(refIdCollectionNameArray[index_masterId_arr], collection_name, strlen(collection_name));

        // Obtain the quantity of items in the collection
        int arrsize = cJSON_GetArraySize(items);

        for (int j = 0; j < arrsize; j++)
        {
            // Obtain the URL and the master_id of the image
            cJSON *imagedata = cJSON_GetObjectItemCaseSensitive(cJSON_GetArrayItem(items, j), "image");
            cJSON *tile = cJSON_GetObjectItemCaseSensitive(imagedata, "tile");
            cJSON *data178 = cJSON_GetObjectItemCaseSensitive(tile, "1.78");
            char *master_id = data178->child->child->child->valuestring;
            char *url = data178->child->child->child->next->next->next->valuestring;

            // Store the master_id
            strncpy(masterIdsArray[index_masterId_arr][j], master_id, strlen(master_id));

            // Download the image
            download_image(master_id, url, dir_path);
        }
    }
    else
    {
        printf("Failed to get JSON Data.\n");
    }
    cJSON_Delete(data);
}

// Handle the information of home.json
void obtainInfoHomeJson(cJSON *homeJson)
{
    // Obtain data{}
    cJSON *data = cJSON_GetObjectItemCaseSensitive(homeJson, "data");
    if (data != NULL)
    {
        // Obtain containers collection
        cJSON *containers = cJSON_GetObjectItemCaseSensitive(data->child, "containers");

        // Obtain quantity of items
        int arrsize = cJSON_GetArraySize(containers);

        int refId_index = 0;
        for (int i = 0; i < arrsize; i++)
        {
            cJSON *item = cJSON_GetArrayItem(containers, i);

            // Obtain the ref_id of the item
            char *str_refid = item->child->child->next->string;
            if (strncmp(str_refid, text_refId, strlen(str_refid)) == 0)
            {
                char *str_val_refid = item->child->child->next->valuestring;

                // Store the ref_id
                strncpy(refIdsArray[refId_index], str_val_refid, strlen(str_val_refid));
                refId_index++;
            };
        }
    }
    else
    {
        printf("Failed to get JSON Data.\n");
    }
    cJSON_Delete(data);
}

// Displays an initial image when menu is loading
void renderDisneyLoading()
{

    SDL_Surface *surface = IMG_Load("assets/disneyplusbegin.jpg");
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    SDL_Rect rect = {240, 50, 1440, 810};
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);
    SDL_Rect load_rect = {800, 860, 10 * strlen(loadingtext), 30};
    SDL_Surface *textsurface = TTF_RenderText_Shaded(font, loadingtext, white, black);
    SDL_Texture *loadingtext = SDL_CreateTextureFromSurface(renderer, textsurface);
    SDL_FreeSurface(textsurface);
    SDL_RenderCopy(renderer, loadingtext, NULL, &load_rect);
    SDL_DestroyTexture(loadingtext);
    SDL_RenderPresent(renderer);
}

//Displays the menu
void renderMenu()
{
    // Render the images
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            int col = j % COLS;
            int x = (col * IMAGE_WIDTH + 50 * (col + 1));
            int y = (i * IMAGE_HEIGHT + 75 * (i + 1));
            SDL_Rect image_rect = {x, y, IMAGE_WIDTH, IMAGE_HEIGHT};
            SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, image_surfaces[i][j]);

            SDL_RenderCopy(renderer, texture, NULL, &image_rect);
            SDL_DestroyTexture(texture);
        }
    }

    // Render the names of the rows
    for (int i = 0; i < ROWS; i++)
    {
        // Loading the name of rows to show
        int x = 50;
        int y = (i * IMAGE_HEIGHT + 75 * (i + 1)) - 35;
        SDL_Rect row_name_rect = {x, y, 10 * strlen(refIdCollectionNameArray[i + ofset_row_names]), 24};
        rowname_surfaces[i] = TTF_RenderText_Shaded(font, refIdCollectionNameArray[i + ofset_row_names], white, black);
        SDL_Texture *textureNames = SDL_CreateTextureFromSurface(renderer, rowname_surfaces[i]);
        SDL_RenderCopy(renderer, textureNames, NULL, &row_name_rect);
        SDL_DestroyTexture(textureNames);
    }

    // Draw a white rectangle around the selected image
    int row = selected_image / COLS;
    int col = selected_image % COLS;
    int x = (col * IMAGE_WIDTH + 50 * (col + 1));
    int y = row * IMAGE_HEIGHT + 75 * (row + 1);
    // Leave a space between the rectangle and the image border
    SDL_Rect highlight_rect = {x - 10, y - 10, IMAGE_WIDTH + 20, IMAGE_HEIGHT + 20};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &highlight_rect);

    // Present the renderer
    SDL_RenderPresent(renderer);
}

// Logic for scrolling down the menu
void updateScrollDown(int row, int ofset_horizontal)
{
    int index = 0;
    for (int i = 0; i < ROWS - 1; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            image_surfaces[i][j] = image_surfaces[i + 1][j];
        }
        index = i;
    }

    // Construct the refID path
    char *refIdPATH = buildRefIDPath(row);

    // Modify images to be shown
    for (int j = 0; j < COLS; j++)
    {
        char *finalpath = buildImageFullPath(refIdPATH, row, j + ofset_horizontal);
        image_surfaces[index + 1][j] = IMG_Load(finalpath);
    }

    // Increment the ofset for changing the name of the rows
    ofset_row_names++;
}

// Logic for scrolling up the menu
void updateScrollUp(int row, int ofset_horizontal)
{
    int index = 0;
    for (int i = 0; i < ROWS - 1; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            image_surfaces[ROWS - i - 1][j] = image_surfaces[ROWS - i - 2][j];
        }
        index = i;
    }

    // Construct the refID path
    char *refIdPATH = buildRefIDPath(row);

    // Modify images to be shown
    for (int j = 0; j < COLS; j++)
    {
        char *finalpath = buildImageFullPath(refIdPATH, row, j + ofset_horizontal);
        image_surfaces[ROWS - index - 2][j] = IMG_Load(finalpath);
    }

    // Less the ofset for changing the name of the rows
    ofset_row_names--;
}

// Logic for scrolling horizontally each row in the menu
void updateSurfaces(int row, int ofset_vertical, int col)
{
    // Construct the refID path
    char *refIdPATH = buildRefIDPath(row + ofset_vertical);

    // Modify images to be shown
    for (int j = 0; j < COLS; j++)
    {
        char *finalpath = buildImageFullPath(refIdPATH, row + ofset_vertical, col - COLS + 1 + j);

        image_surfaces[row][j] = IMG_Load(finalpath);
    }
}