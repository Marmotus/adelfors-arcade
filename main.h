#include "C:/msys64/mingw64/include/SDL2/SDL.h"
#include "C:/msys64/mingw64/include/SDL2/SDL_ttf.h"
#include "C:/msys64/mingw64/include/SDL2/SDL_image.h"
#include "C:/msys64/mingw64/include/SDL2/SDL_syswm.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <windows.h>



#define TICK_RATE 1000 / 30

#define ARCADE_UI_EDGE_MARGIN 100

const SDL_Color TEXT_COLOR = {255, 255, 255, 255};
const SDL_Color BACKGROUND_COLOR = {0, 0, 0, 255};
const SDL_Color FOREGROUND_COLOR = {28, 28, 28, 255};
const SDL_Color SELECTION_COLOR = {80, 80, 80, 255};

TTF_Font* font_big = NULL;
TTF_Font* font_medium = NULL;

SDL_Texture* game_name_texture = NULL;
SDL_Texture* run_game_hint_texture = NULL;
SDL_Texture* loading_text_texture = NULL;
SDL_Texture* no_games_text_texure = NULL;

SDL_Surface* button_pink_surface = NULL;
SDL_Texture* button_pink_texture = NULL;

typedef struct {
  char* game_title;
  char* exe_path;
} GameEntry;

typedef struct {
  int x;
  int y;
} Position;

typedef enum {
  LOADING = 0,
  GAME_SELECT = 1,
  SHUTDOWN = 2,
} MenuState;

typedef struct {
  SDL_Window* window;
  SDL_Renderer* renderer;
  HWND window_handle;
  HANDLE game_process_handle;
  int window_w;
  int window_h;
  int rows;
  int columns;
  Position selection;
  int page;
  GameEntry* game_entries;
  int game_entries_len;
  char input_enabled;
  SDL_Joystick* joystick;
  MenuState menu_state;
} State;

int load_font(State* state, const char* font_path);

void free_state(State* state);
void free_game_entries(State* state);

int find_games(State* state);
DWORD WINAPI start_game_thread(void* data);
void run_selected_game(State* state);

int handle_events(State* state);

void render(State* state);

void render_loading_ui(State* state);
void render_game_select_ui(State* state);
void render_shutdown_ui(State* state);

void move_select_up(State* state);
void move_select_down(State* state);
void move_select_left(State* state);
void move_select_right(State* state);

int get_real_selection_index(State* state);

void generate_new_game_name(State* state);
