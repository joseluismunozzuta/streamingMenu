
This is an exercise given by Disney+ recruitment team in order to evaluate candidates technical skills. It is about developing a type of streaming menu that allows interaction with the user by keyboard. It must look pretty similar to:

![image](https://user-images.githubusercontent.com/58311946/220862087-bb22014f-3c09-4f89-8bbb-8c88e7847879.png)

For developing it, C language is going to be used in a Unix OS. Furthermore, 3rd party libraries are going to be used. Here is the list of libraries used and why are they chosen:
  - SDL: This library is used for rendering and drawing.
  - SDL_image: This library is used to render images. As images are going to be saved as files, it's necessary to load them for rendering.
  - cJSON: This library is used to parse JSON files.
  - curl: This is a library that allows to transfer data using network protocols. This is used for dowloading the images from the URL. Furthermore, as an API must be consumed, curl will be helpful when doing the request for reading the JSON objects.
  
SDL2 libraries (SDL, SDL_image, SDL_ttf)

```sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev```

CURL libraries

```sudo apt-get install libcurl4-openssl-dev```

CJSON library

- For users in Ubuntu 20.04 and up, run this command:

```sudo apt-get install libcjson-dev```

- For users in Ubuntu 18.04, follow these steps:

```sudo apt-get install git```

```git clone https://github.com/DaveGamble/cJSON.git```

```cd cJSON```

```make```

```sudo make install```

For compilation:

- Ubuntu >= 20.04

```gcc -o menu menu.c -lSDL2 -lSDL2_image -lSDL2_ttf -I/usr/include/cjson -lcjson -lcurl```

- Ubuntu 18.04

```gcc -o menu menu.c -lSDL2 -lSDL2_image -lSDL2_ttf -I/usr/local/include/cjson -L/usr/lib/x86_64-linux-gnu -lcjson -lcurl```