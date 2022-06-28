#include <stdio.h>
#include <stdbool.h>

#include "include/SDL.h" // připojení SDL knihovny (pozor pokud není ve stejném adresáři musí se připovit včetně adresáře <SDL2/SDL.h>)
//#include <SDL2/SDL.h>
#include "./constants.h" // pokud si vytvořím svuj sobor můžu ho tímto připojit.

#include  "include/SDL_ttf.h"
//#include <SDL2/SDL_ttf.h>

int game_is_running = false;
int move_right = false;
int move_left = false;
int start_game = false;

int score = 0;
int life = 3;

char konec[10] = "Game Over";
char win[10] = "Winner";
int block_number;
int block_pocet;

float * block_position;


SDL_Window* window = NULL; // definovaná struktura pro proměnou OKno 
SDL_Renderer* renderer = NULL;
SDL_Texture  *texture2, *texture, *texture3, *texture4;
SDL_Surface *surface, *life_surface, *konec_surface, *win_surface;


int last_frame_time = 0;


struct game_object {
    float x;
    float y;
    float width;
    float height;
    float vel_x;
    float vel_y;
}ball, paddle;

struct border {
    float x;
    float y;
    float width;
    float height;
}border_left, border_top, border_right, score_display, life_display, konec_display, win_display;


struct kostka {
    float x;
    float y;
    float width;
    float height;
}kostka[100];
/*
typedef struct{
    float x, y, width, height;
}Block;

Block *block[MAX_BLOCK] = {NULL};*/

int initialize_window() // funkce na otevření "zavolání okna"
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) // funkce knihovny SDL, která zavolá všechny komponenty, pokud neznám ten konkrétní který potřebuji. Je zabalená v IF pro případ chyby
    {
        fprintf(stderr, "Error initializing SDL ");
        return  false;
    }

     window = SDL_CreateWindow(         // první parametr je titulek, druhž parametr je pozice x,y kde se okno objeví, další parametr je šířka a výška okna, poslední parametr je něco pro vzled např (full scren)
         "Game", // titulek
         SDL_WINDOWPOS_CENTERED,  //x - funkce SDL, která okno objeví uprostřed
         SDL_WINDOWPOS_CENTERED,  //y
         Window_Width,  // proměné definované v soboru constants.h
         Window_Height,
         //SDL_WINDOW_BORDERLESS // bude to okno bez ohraničení
         0
         );

    if(!window)
    {
        fprintf(stderr, "Error create window");
        return false;
    }

    // po vytvoření okna musíme zavolat render, který objekt vykreslí 
    // první parametr je co budeme vykreslovat, duhý je driver, když dáme -1 je to defaultní, poslední je speciální požadavek 
   renderer = SDL_CreateRenderer(window, -1, 0);

   if(!renderer)
   {
       fprintf(stderr, "Error create renderer");
       return false;
   }
    return true; //  definované hodnoty v souboru constants.h
}

void process_input() // vstupy z klávesnice
{
    SDL_Event event;
    SDL_PollEvent(&event);

    switch(event.type) 
    {
        case SDL_QUIT:
            game_is_running = false; 
            break;
        //SDL_QUIT je tlačítko X na okně
        case SDL_KEYDOWN:
            if(event.key.keysym.sym == SDLK_ESCAPE)// zmáčknutí na klávesnici
                game_is_running = false;
            // dodělat pohyb šipkama
            if(event.key.keysym.sym == SDLK_LEFT) 
                move_left = true;
            if(event.key.keysym.sym == SDLK_RIGHT)
                move_right = true;
            if(event.key.keysym.sym == SDLK_SPACE)
                start_game = true;
                break;
                 // resetovat pohyb šipek 
        case SDL_KEYUP:
            if(event.key.keysym.sym == SDLK_LEFT) 
                move_left = false;
            if(event.key.keysym.sym == SDLK_RIGHT)
                move_right = false;
            if(event.key.keysym.sym == SDLK_SPACE)
                start_game = false;
            break;
    }

}

void setup() // nastavení počátečních hodnot
{
    //nactení soubor s bloky

    FILE* soubor = fopen("blok.txt", "r");
    if(soubor == NULL)
    { fprintf(stderr, "error: file not found");
      exit(EXIT_FAILURE);    
    }

    fscanf(soubor, "%d", &block_number);
    
    block_pocet = block_number * 2;

    block_position = malloc((block_pocet) * sizeof(float));

    for(int i = 0; i < block_pocet; i++)
       {
         fscanf(soubor, "%f", &block_position[i]);
       }

    fclose(soubor);
    //startovací  parametry 
    ball.x = 300;
    ball.y = 700;
    ball.width = 15;
    ball.height = 15;
    ball.vel_x = 0;
    ball.vel_y = 0;

    border_left.x = 0;
    border_left.y = 50;
    border_left.width = 15;
    border_left.height = 800;

    border_top.x = 0;
    border_top.y = 50;
    border_top.width = 600;
    border_top.height = 30;

    border_right.x = 585;
    border_right.y = 50;
    border_right.width = 15;
    border_right.height = 800;

    paddle.x = 275;
    paddle.y = 715;
    paddle.width = 50;
    paddle.height = 15;
    paddle.vel_x = 300;
    paddle.vel_y = 0;

    score_display.x = 100;
    score_display.y = 5;
    score_display.width = 60;
    score_display.height = 40; 

    life_display.x = 400;
    life_display.y = 5;
    life_display.width = 60;
    life_display.height = 40; 

    konec_display.x = 225;
    konec_display.y = 400;
    konec_display.width = 150;
    konec_display.height = 40; 

    win_display.x = 225;
    win_display.y = 400;
    win_display.width = 150;
    win_display.height = 40; 

for(int i = 0; i < block_pocet-1; i = i+2 )
    {
    kostka[i].x = block_position[i];
    kostka[i].y = block_position[i+1];
    kostka[i].width = 57;
    kostka[i].height = 30;
    } 
}
void destroy_block() // zničení bloku
{
 for(int i = 0; i <  block_pocet; i++)
{   
    if((ball.y + ball.height) >= kostka[i].y && (ball.y + ball.height) <= (kostka[i].y + kostka[i].height) && (ball.x + ball.width) >= kostka[i].x && (ball.x + ball.width) <= (kostka[i].x+kostka[i].width))
    {
        ball.vel_y = -ball.vel_y;
        kostka[i].width = 0;
        kostka[i].height = 0;
        kostka[i].x = 0;
        kostka[i].y = 0; 
        score += 1;
    }
}   
    
}
void game_over() // konec hry a reset po ubrání života, a také konec po vyhrání hry
{
    if(ball.y + ball.height > Window_Height && life>=0)
    {
    ball.x = 300;
    ball.y = 700;
    ball.vel_x = 0;
    ball.vel_y = 0;

    paddle.x = 275;
    paddle.y = 715;

    life -= 1;
    }
    else if(ball.y + ball.height > Window_Height)
    {
    ball.x = 300;
    ball.y = 700;
    ball.vel_x = 0;
    ball.vel_y = 0;

    paddle.x = 275;
    paddle.y = 715; 
    }

    if(score == block_number)
    {
    ball.x = 300;
    ball.y = 700;
    ball.vel_x = 0;
    ball.vel_y = 0;

    paddle.x = 275;
    paddle.y = 715; 
    }
}
void reset_game() // znovu spuštění hry po ubrání života
{
    if(start_game == true && life>=0)
    {
    ball.vel_x = 30;
    ball.vel_y = -500;
    }
}


void update()
{
    // udělání zpoždení mezi posledním snímkem a časem kolik snímku chceme za milisekundu
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks()-last_frame_time);

    if(time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME)
    {
        SDL_Delay(time_to_wait);
    }

    float delta_time = (SDL_GetTicks() - last_frame_time) /1000.0;
// SDL GET Ticks  - aktuální hodnota milisekund
    last_frame_time = SDL_GetTicks(); // kolik milisendund mezi snímky
// POhyb míčku 

    ball.x += ball.vel_x * delta_time;
    ball.y += ball.vel_y * delta_time;

// Pohyb plošinou 
    if(move_left == true)
     {   
        paddle.x -= paddle.vel_x * delta_time;
     }
    if(move_right == true)
    {
        paddle.x += paddle.vel_x * delta_time;
    }
// odražení od okrajů 
if(ball.x <= 15 || ball.x + ball.width >= Window_Width - 15)
{
    ball.vel_x =  -ball.vel_x;
}
if(ball.y < (border_top.height + border_top.y))
{
    ball.vel_y = -ball.vel_y;
}

//odražení od plošiny
// pro odražení jsme museli zjistit zda se dotýkají na ose y a zda jsou na společné ose x  aby míček nebyl bokem
if(ball.y + ball.height >= paddle.y && ball.x + ball.width >= paddle.x && ball.x <= paddle.x + paddle.width)
{
    ball.vel_y = -ball.vel_y;
}
// ošetření aby nešlo plošinou vyjet z obrazovky
if(paddle.x <= 0 + border_left.width)
{
    paddle.x =0 + border_left.width;
}
if(paddle.x >= Window_Width - paddle.width - border_right.width)
{
    paddle.x = Window_Width-paddle.width - border_right.width;
}

destroy_block(); 
game_over();
reset_game();
}

void render(TTF_Font *font) // vykreslení objektu
{
    //pozadí
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);//RGB barvy
    SDL_RenderClear(renderer);

    //balonek
    SDL_Rect ball_rect = {
        (int)ball.x,
        (int)ball.y,
        (int)ball.width,
        (int)ball.height };

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &ball_rect);

    // ohraničení
    SDL_Rect border_left_rect ={
        border_left.x,
        border_left.y,
        border_left.width,
        border_left.height };

    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
    SDL_RenderFillRect(renderer, &border_left_rect);

     SDL_Rect border_top_rect ={
        border_top.x,
        border_top.y,
        border_top.width,
        border_top.height };

    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
    SDL_RenderFillRect(renderer, &border_top_rect);

     SDL_Rect border_right_rect ={
        border_right.x,
        border_right.y,
        border_right.width,
        border_right.height };

    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
    SDL_RenderFillRect(renderer, &border_right_rect);

    SDL_Rect paddle_rect ={
        paddle.x,
        paddle.y,
        paddle.width,
        paddle.height };

    SDL_SetRenderDrawColor(renderer, 255, 10, 10, 255);
    SDL_RenderFillRect(renderer, &paddle_rect);


  // vykreslení kostek  
    
    for(int i = 0; i < 18; i++)
    {
        SDL_Rect rect = {kostka[i].x, kostka[i].y, kostka[i].width, kostka[i].height} ;
        SDL_SetRenderDrawColor(renderer, 20, 150, 200, 255);
        SDL_RenderFillRect(renderer, &rect);
    }   
    for(int i = 18; i < 35; i++)
    {
        SDL_Rect rect = {kostka[i].x, kostka[i].y, kostka[i].width, kostka[i].height} ;
        SDL_SetRenderDrawColor(renderer, 220, 150, 200, 255);
        SDL_RenderFillRect(renderer, &rect);
    }  
    for(int i = 35; i < 53; i++)
    {
        SDL_Rect rect = {kostka[i].x, kostka[i].y, kostka[i].width, kostka[i].height} ;
        SDL_SetRenderDrawColor(renderer, 220, 250, 200, 255);
        SDL_RenderFillRect(renderer, &rect);
    }  
    for(int i = 53; i < block_pocet; i++)
    {
        SDL_Rect rect = {kostka[i].x, kostka[i].y, kostka[i].width, kostka[i].height} ;
        SDL_SetRenderDrawColor(renderer, 10, 250, 200, 255);
        SDL_RenderFillRect(renderer, &rect);
    }  

  // Text 
    // převod Intu na Char
    char snum[5];
    snprintf(snum, 5, "%d", score);

    char slife[5];
    snprintf(slife, 5, "%d", life);

    SDL_Color textColor = {255,255,255,0};
   // Skore
    surface = TTF_RenderText_Solid(font,  snum, textColor);
    texture2 = SDL_CreateTextureFromSurface(renderer, surface);


    SDL_Rect score_display_rect={
        score_display.x,
        score_display.y,
        score_display.width,
        score_display.height}; 
    
    
    SDL_RenderCopy(renderer, texture2, NULL, &score_display_rect);
    // životy
    life_surface = TTF_RenderText_Solid(font,  slife, textColor);
    texture = SDL_CreateTextureFromSurface(renderer, life_surface);
    
    SDL_Rect life_display_rect={
        life_display.x,
        life_display.y,
        life_display.width,
        life_display.height};

    SDL_RenderCopy(renderer, texture, NULL, &life_display_rect);
    //Game Over
    if(life < 0)
    {
    konec_surface = TTF_RenderText_Solid(font,  &konec[0], textColor);
    texture3 = SDL_CreateTextureFromSurface(renderer, konec_surface);

    SDL_Rect konec_display_rect={
        konec_display.x,
        konec_display.y,
        konec_display.width,
        konec_display.height};

    SDL_RenderCopy(renderer, texture3, NULL, &konec_display_rect);
    }

    if(score == block_number)
    {
        win_surface = TTF_RenderText_Solid(font,  &win[0], textColor);
        texture4 = SDL_CreateTextureFromSurface(renderer, win_surface);
    
        SDL_Rect win_display_rect={
            win_display.x,
            win_display.y,
            win_display.width,
            win_display.height};

        SDL_RenderCopy(renderer, texture4, NULL, &win_display_rect);
    }

    SDL_FreeSurface(surface);
    SDL_FreeSurface(life_surface);
    SDL_FreeSurface(konec_surface);
    SDL_FreeSurface(win_surface);
    SDL_RenderPresent(renderer);//back and front buffer
}

void destroy_window() // zničení okna
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main()
{
    game_is_running = initialize_window();
    setup();

    TTF_Init();
    TTF_Font *font = TTF_OpenFont("arial.ttf", 24);
    if (font == NULL) {
        fprintf(stderr, "error: font not found\n");
        exit(EXIT_FAILURE);
    }
    
    while(game_is_running)
    {
        process_input();
        update();
        render(font);
        
    }

    free(block_position);
    destroy_window();
    return 0;
}