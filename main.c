#include "main.h"
#include <dirent.h>
#include <errhandlingapi.h>
#include <errno.h>
#include <handleapi.h>
#include <minwinbase.h>
#include <processthreadsapi.h>
#include <stdio.h>
#include <string.h>
#include <synchapi.h>
#include <sys/stat.h>
#include <windows.h>

int main(int argc, char* argv[])
{
  printf("Starting Adelfors Arcade...\n");

  State state = {NULL, NULL};
  state.game_entries = NULL;
  state.game_entries_len = 0;
  state.rows = 2;
  state.columns = 4;
  state.selection = (Position){0, 0};
  state.page = 0;
  state.game_process_handle = 0;
  state.game_entries_len = 0;
  state.input_enabled = 0;
  state.joystick = NULL;
  state.window_w = 1650;
  state.window_h = 1050;
  state.menu_state = 0;

  printf("Initializing SDL...\n");
  if (SDL_Init(SDL_INIT_EVERYTHING | SDL_INIT_JOYSTICK))
  {
    printf("Error initializing SDL: %s\n", SDL_GetError());
    return 1;
  }

  printf("Initializing TTF...\n");
  if (TTF_Init() != 0)
  {
    printf("Error initializing TTF: %s\n", TTF_GetError());
    return 1;
  }

  printf("Creating window...\n");
  state.window = SDL_CreateWindow("Ädelfors Arcade", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_FULLSCREEN_DESKTOP);
  if (state.window == NULL) return 1;

  printf("Creating renderer...\n");
  state.renderer = SDL_CreateRenderer(state.window, -1, SDL_RENDERER_ACCELERATED);
  if (state.renderer == NULL) return 1;

  // SDL_GetWindowSize(state.window, &state.window_w, &state.window_h);
  SDL_GetRendererOutputSize(state.renderer, &state.window_w, &state.window_h);
  
  // Initial render
  render(&state);
  
  load_font(&state, "./font/font.ttf");

  printf("Loading sprites...\n");
  if ((button_pink_surface = IMG_Load("./images/arcade_button_pink.png")) != NULL)
  {
    if ((button_pink_texture = SDL_CreateTextureFromSurface(state.renderer, button_pink_surface)) == NULL)
    {
      printf("  ERROR: Creating texture for arcade_button_pink failed\n");
    }

    SDL_FreeSurface(button_pink_surface);
  }
  else printf("  ERROR: Failed to load \"arcade_button_pink.png\"\n");

  // Enable joystick input and find first available joystick
  SDL_JoystickEventState(SDL_ENABLE);
  state.joystick = SDL_JoystickOpen(0);
  if (state.joystick == NULL) printf("ERROR: JOYSTICK NULL!\n");
  else printf("Joystick found\n");
  
  printf("Finding games...\n");
  if (find_games(&state) != 0)
  {
    printf("Error finding games\n");
  }

  generate_new_game_name(&state);

  char should_quit = 0;

  printf("Beginning loop...\n");
  
  // Ignore these types of events for performance. We don't need them.
  SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE);
  SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);

  state.menu_state = 1;
  state.input_enabled = 1;
  
  // Initial render
  render(&state);
  
  while (should_quit == 0)
  {
    if (state.menu_state != 0 && state.game_process_handle != NULL)
    {
      CloseHandle(state.game_process_handle);
      state.game_process_handle = NULL;
    }
    
    should_quit = handle_events(&state);

    // Limit loop frequency to leave more resources for the actual games and to save on electricity
    SDL_Delay(TICK_RATE);
  }

  if (game_name_texture != NULL) SDL_DestroyTexture(game_name_texture);
  if (run_game_hint_texture != NULL) SDL_DestroyTexture(run_game_hint_texture);
  if (loading_text_texture != NULL) SDL_DestroyTexture(loading_text_texture);
  if (no_games_text_texure != NULL) SDL_DestroyTexture(no_games_text_texure);
  if (button_pink_texture != NULL) SDL_DestroyTexture(button_pink_texture);
  
  if (font_big != NULL) TTF_CloseFont(font_big);
  if (font_medium != NULL) TTF_CloseFont(font_medium);

  TTF_Quit();

  free_state(&state);
  
  SDL_DestroyRenderer(state.renderer);
  SDL_DestroyWindow(state.window);
  SDL_Quit();

  printf("Shut down successfully!\n");
  
  return EXIT_SUCCESS;
}

int handle_events(State* state)
{
  char should_quit = 0;
  char should_render = 0;

  const Uint8* keyboard_state = SDL_GetKeyboardState(NULL);
  
  SDL_Event event;

  while (SDL_PollEvent(&event))
  {
    if (state->input_enabled != 1) continue;
    
    // Handle events
    switch (event.type)
    {
      case SDL_QUIT:
      {
        should_quit = 1;
        break;
      }
      case SDL_JOYAXISMOTION:
      {
        if (event.jaxis.value < -3200 || event.jaxis.value > 3200)
        {
          int old_index = get_real_selection_index(state);
          
          // Left-right
          if (event.jaxis.axis == 0)
          {
            if (event.jaxis.value > 0)
            {
              if (state->menu_state == 1) move_select_right(state);
            }
            else if (event.jaxis.value < 0)
            {
              if (state->menu_state == 1) move_select_left(state);
            }
          }
          
          // Up-down
          if (event.jaxis.axis == 1)
          {
            if (event.jaxis.value > 0)
            {
              if (state->menu_state == 1) move_select_down(state);
            }
            else if (event.jaxis.value < 0)
            {
              if (state->menu_state == 1) move_select_up(state);
            }
          }

          if (get_real_selection_index(state) != old_index)
          {
            generate_new_game_name(state);
          }

          should_render = 1;
        }
        
        break;
      }

      case SDL_JOYBUTTONDOWN:
      {
        switch (event.jbutton.button)
        {
          // Pink button
          case 0:
          {
            if (state->menu_state == 1)
            {
              if (state->game_process_handle == NULL) state->game_process_handle = CreateThread(NULL, 0, start_game_thread, state, 0, NULL);
            }
            break;
          }
          // White button
          case 1:
          {
            break;
          }
          // Yellow button
          case 2:
          {
            break;
          }
          // Blue button
          case 3:
          {
            break;
          }
          // Right black button
          case 6:
          {
            if (state->menu_state == 1)
            {
              //
            }
            // else if (state->menu_state == 2)
            // {
            //   if (system("C:\\Windows\\System32\\shutdown /s /t 0 /d u:0:0") != 0) printf("ERROR: Shutdown failed\n");
            //   else printf("Shutting down...\n");
            // }
            
            break;
          }
          // Left black button
          case 7:
          {
            // if (state->menu_state == 1) state->menu_state = 2;
            // else if (state->menu_state == 2) state->menu_state = 1;

            // should_render = 1;
            
            // printf("'Cancel' pressed\n");
            break;
          }

          default:
          {
            break;
          }
        }
        
        break;
      }
      case SDL_KEYDOWN:
      {
        int old_index = get_real_selection_index(state);
        
        switch (event.key.keysym.scancode)
        {
          case SDL_SCANCODE_UP:
          {
            move_select_up(state);
            break;
          }
          case SDL_SCANCODE_DOWN:
          {
            move_select_down(state);
            break;
          }
          case SDL_SCANCODE_LEFT:
          {
            move_select_left(state);
            break;
          }
          case SDL_SCANCODE_RIGHT:
          {
            move_select_right(state);
            break;
          }

          case SDL_SCANCODE_RETURN:
          {
            if (state->menu_state == 1)
            {
              if (state->game_process_handle == NULL) state->game_process_handle = CreateThread(NULL, 0, start_game_thread, state, 0, NULL);
            }
          }

          case SDL_SCANCODE_P:
          {
            if (state->menu_state != 1) break;
            
            // If Left Shift and R are held down when P is pressed
            if (keyboard_state[SDL_SCANCODE_LSHIFT] == 1 && keyboard_state[SDL_SCANCODE_R] == 1)
            {
              state->input_enabled = 0;
              state->menu_state = 0;
              render(state);
              
              printf("Searching for games...\n");
              find_games(state);
              printf("Search done\n");

              state->menu_state = 1;
              state->input_enabled = 1;
              render(state);
            }
          }

          default:
          {
            break;
          }
        }
        
        if (get_real_selection_index(state) != old_index)
        {
          generate_new_game_name(state);
          should_render = 1;
        }
        
        break;
      }
      
      default:
      {
        break;
      }
    }
  }

  if (should_render == 1) render(state);
  
  return should_quit;
}

void render(State* state)
{
  SDL_SetRenderDrawColor(state->renderer, BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b, BACKGROUND_COLOR.a);
  SDL_RenderClear(state->renderer);

  if (state->menu_state == 0) render_loading_ui(state);
  else if (state->menu_state == 1) render_game_select_ui(state);
  else if (state->menu_state == 2) render_shutdown_ui(state);

  SDL_RenderPresent(state->renderer);
}

void render_loading_ui(State* state)
{
  if (state == NULL)
  {
    printf("ERROR: render_loading_ui: state is NULL");
    return;
  }

  SDL_Rect text_rect = {0, 0, 0, 0,};
  SDL_QueryTexture(loading_text_texture, NULL, NULL, &text_rect.w, &text_rect.h);
  text_rect.x = state->window_w / 2 - text_rect.w / 2;
  text_rect.y = state->window_h / 2 - text_rect.h / 2;

  SDL_RenderCopy(state->renderer, loading_text_texture, NULL, &text_rect);
}

void render_game_select_ui(State* state)
{
  // Pre-calculate some values to make it easier to render game boxes
  const int left = 200;
  const int top = 250;
  const int bottom = (state->window_h) - 200;
  const int w = state->window_w - (left * 2);
  const int h = bottom - top;
  
  if (state->game_entries != NULL && state->game_entries_len > 0)
  {
    int count = 0;
  
    for (int row = 0; row < state->rows; row++)
    {
      for (int column = 0; column < state->columns; column++)
      {
        if (count + 1 > state->game_entries_len - (state->rows * state->columns) * (state->page)) break;
      
        if (row == state->selection.y && column == state->selection.x)
        {
          SDL_SetRenderDrawColor(state->renderer, SELECTION_COLOR.r, SELECTION_COLOR.g, SELECTION_COLOR.b, SELECTION_COLOR.a);
        }
        else
        {
          SDL_SetRenderDrawColor(state->renderer, FOREGROUND_COLOR.r, FOREGROUND_COLOR.g, FOREGROUND_COLOR.b, FOREGROUND_COLOR.a);
        }
      
        SDL_Rect rect = (SDL_Rect){
          left + (w / state->columns) * column,
          top + (h / state->rows) * row,
          w / state->columns,
          h / state->rows,
        };
        SDL_RenderFillRect(state->renderer, &rect);

        count++;
      }
    }
    
    if (button_pink_texture != NULL)
    {
      SDL_Rect btn_pink_rect = {0, 0, 0, 0};
      SDL_QueryTexture(button_pink_texture, NULL, NULL, &btn_pink_rect.w, &btn_pink_rect.h);

      SDL_Rect pos_rect = {state->window_w - 500, state->window_h - 100, 75, 75};

      SDL_RenderCopy(state->renderer, button_pink_texture, NULL, &pos_rect);
    }

    if (run_game_hint_texture != NULL)
    {
      SDL_Rect text_rect = {0, 0, 0, 0,};
      SDL_QueryTexture(run_game_hint_texture, NULL, NULL, &text_rect.w, &text_rect.h);
      text_rect.x = state->window_w - text_rect.w - 50;
      text_rect.y = state->window_h - 75;

      SDL_RenderCopy(state->renderer, run_game_hint_texture, NULL, &text_rect);
    }

    if (game_name_texture != NULL)
    {
      SDL_Rect text_rect = {0, 0, 0, 0};
      SDL_QueryTexture(game_name_texture, NULL, NULL, &text_rect.w, &text_rect.h);
      text_rect.x = state->window_w / 2 - text_rect.w / 2;
      text_rect.y = 50 + text_rect.h;
    
      SDL_RenderCopy(state->renderer, game_name_texture, NULL, &text_rect);
    }
  }
  else
  {
    SDL_Rect text_rect = {0, 0, 0, 0};
    SDL_QueryTexture(no_games_text_texure, NULL, NULL, &text_rect.w, &text_rect.h);
    text_rect.x = state->window_w / 2 - text_rect.w / 2;
    text_rect.y = state->window_h / 2 - text_rect.h / 2;

    SDL_RenderCopy(state->renderer, no_games_text_texure, NULL, &text_rect);
  }
}

void render_shutdown_ui(State* state)
{
  SDL_SetRenderDrawColor(state->renderer, 28, 28, 28, 255);

  SDL_Rect rect = {ARCADE_UI_EDGE_MARGIN * 2, ARCADE_UI_EDGE_MARGIN, state->window_w - ARCADE_UI_EDGE_MARGIN * 4, state->window_h - ARCADE_UI_EDGE_MARGIN * 2};
  SDL_RenderFillRect(state->renderer, &rect);
}

int load_font(State* state, const char* font_path)
{
  printf("Loading font...\n");

  if (font_path != NULL && strlen(font_path) > 1)
  {
    font_big = TTF_OpenFont(font_path, 48);
    if (font_big == NULL)
    {
      printf("ERROR: Failed loading font\n");
      return 1;
    }

    font_medium = TTF_OpenFont(font_path, 32);
    if (font_medium == NULL)
    {
      printf("ERROR: Failed loading font\n");
      return 1;
    }

    SDL_Surface* run_game_hint_surface = TTF_RenderUTF8_Solid(font_medium, "Starta spel", (SDL_Color){255, 255, 255, 150});
    if (run_game_hint_surface == NULL)
    {
      printf("ERROR: Failed to create surface for run game hint: %s\n", TTF_GetError());
      return 1;
    }
    else
    {
      if (run_game_hint_texture != NULL) SDL_DestroyTexture(run_game_hint_texture);
      run_game_hint_texture = SDL_CreateTextureFromSurface(state->renderer, run_game_hint_surface);
      SDL_FreeSurface(run_game_hint_surface);
      if (run_game_hint_texture == NULL)
      {
        printf("ERROR: Failed to create texture for run game hint: %s\n", SDL_GetError());
        return 1;
      }
    }

    SDL_Surface* loading_text_surface = TTF_RenderUTF8_Solid(font_big, "LOADING...", TEXT_COLOR);
    if (loading_text_surface == NULL)
    {
      printf("ERROR: Failed to create surface for loading text: %s\n", TTF_GetError());
      return 1;
    }
    else
    {
      if (loading_text_texture != NULL) SDL_DestroyTexture(loading_text_texture);
      loading_text_texture = SDL_CreateTextureFromSurface(state->renderer, loading_text_surface);
      SDL_FreeSurface(loading_text_surface);
      if (loading_text_texture == NULL)
      {
        printf("ERROR: Failed to create texture for loading text: %s\n", SDL_GetError());
        return 1;
      }
    }

    SDL_Surface* no_games_text_surface = TTF_RenderUTF8_Solid(font_big, "No games found", TEXT_COLOR);
    if (no_games_text_surface == NULL)
    {
      printf("ERROR: Failed to create surface for no games text: %s\n", TTF_GetError());
      return 1;
    }
    else
    {
      if (no_games_text_texure != NULL) SDL_DestroyTexture(no_games_text_texure);
      no_games_text_texure = SDL_CreateTextureFromSurface(state->renderer, no_games_text_surface);
      SDL_FreeSurface(no_games_text_surface);
      if (no_games_text_texure == NULL)
      {
        printf("ERROR: Failed to create texture for no games text: %s\n", SDL_GetError());
        return 1;
      }
    }
  }

  return 0;
}

void free_state(State* state)
{
  if (state->game_entries != NULL)
  {
    free_game_entries(state);
  }
  if (state->joystick != NULL)
  {
    SDL_JoystickClose(state->joystick);
    state->joystick = NULL;
  }
}

void free_game_entries(State* state)
{
  if (state != NULL)
  {
    if (state->game_entries != NULL)
    {
      for (int i = 0; i < state->game_entries_len; i++)
      {
        if (state->game_entries[i].game_title != NULL)
        {
          free(state->game_entries[i].game_title);
          state->game_entries[i].game_title = NULL;
        }
        if (state->game_entries[i].exe_path != NULL)
        {
          free(state->game_entries[i].exe_path);
          state->game_entries[i].exe_path = NULL;
        }
      }
  
      free (state->game_entries);
      state->game_entries = NULL;
      state->game_entries_len = 0;
    }
  }
}

int find_games(State* state)
{
  const char GAMES_PATH[] = "./games";
  
  DIR* dir = opendir(GAMES_PATH);

  if (dir != NULL)
  {
    free_game_entries(state);
    
    struct dirent* dirp;

    while ((dirp = readdir(dir)) != NULL)
    {
      // Ignore current and parent directory links and the desktop.ini file that can appear on Windows
      if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0 || strcmp(dirp->d_name, "desktop.ini") == 0) continue;

      int dirp_name_len = strlen(dirp->d_name);

      char* path = malloc(strlen(GAMES_PATH) + dirp_name_len + 2);
      strcpy(path, GAMES_PATH);
      strcat(path, "/");
      strcat(path, dirp->d_name);
        
      struct stat s;
      if (stat(path, &s) == 0)
      {
        if (s.st_mode & S_IFDIR)
        {
          // Open the folder and look for a .exe
          DIR* game_dir = opendir(path);
          
          if (game_dir != NULL)
          {
            struct dirent* dirp2;
            while((dirp2 = readdir(game_dir)) != NULL)
            {
              if (strcmp(dirp2->d_name, ".") == 0 || strcmp(dirp2->d_name, "..") == 0 || strcmp(dirp2->d_name, "desktop.ini") == 0) continue;
              
              int name_len = strlen(dirp2->d_name);
              if (name_len > 4)
              {
                if (dirp2->d_name[name_len - 4] == '.' && dirp2->d_name[name_len - 3] == 'e' && dirp2->d_name[name_len - 2] == 'x' && dirp2->d_name[name_len - 1] == 'e')
                {
                  printf("  [%s]\n", dirp->d_name);

                  state->game_entries_len++;

                  state->game_entries = (GameEntry*)realloc(state->game_entries, sizeof(GameEntry) * state->game_entries_len);
                  if (state->game_entries == NULL)
                  {
                    printf("  ERROR: Failed to reallocate memory for game_entries\n");
                    break;
                  }
                  state->game_entries[state->game_entries_len - 1].game_title = malloc(dirp_name_len + 1);
                  strcpy(state->game_entries[state->game_entries_len - 1].game_title, dirp->d_name);

                  char* exe_path = malloc(strlen(path) + name_len + 3);
                  if (exe_path == NULL)
                  {
                    printf("  ERROR: Failed to allocate memory for exe_path\n");
                    state->game_entries[state->game_entries_len - 1].exe_path = NULL;
                    break;
                  }

                  strcpy(exe_path, "\"");
                  strcat(exe_path, path);
                  strcat(exe_path, "/");
                  strcat(exe_path, dirp2->d_name);
                  strcat(exe_path, "\"");

                  state->game_entries[state->game_entries_len - 1].exe_path = exe_path;

                  printf("    %s\n", exe_path);
                  
                  break;
                }
              }
            }

            closedir(game_dir);
          }
        }
      }
      else
      {
        printf("  Error reading file\n");
      }

      if (path != NULL) free(path);
    }
    
    closedir(dir);
    return 0;
  }
  else
  {
    switch (errno)
    {
      case EACCES:
        printf("find_games: Permission denied\n");
        break;
      case ENOENT:
        printf("find_games: Directory does not exist\n");
        break;
      case ENOTDIR:
        printf("find_games: Not a directory\n");
        break;
    }
    return 1;
  }
}

DWORD WINAPI start_game_thread(void* data)
{
  State* state = (State*)data;
  if (state == NULL) return 0;

  state->input_enabled = 0;
  run_selected_game(state);
  state->input_enabled = 1;
  return 0;
}

void run_selected_game(State* state)
{
  if (state->menu_state == 1)
  {
    if (state->game_entries != NULL)
    {
      int sel_idx = get_real_selection_index(state);
      if (state->game_entries[sel_idx].exe_path != NULL)
      {
        printf("\nLaunching %s...\n\n", state->game_entries[sel_idx].game_title);
        
        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        if (!CreateProcess(NULL, state->game_entries[sel_idx].exe_path, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        {
          printf("ERROR: Failed to launch game. CreateProcess failed: %lu\n", GetLastError());
          return;
        }

        // If game launched
        state->menu_state = 0;
        render(state);

        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        
        state->menu_state = 1;

        printf("\n\n%s exited\n\n", state->game_entries[sel_idx].game_title);

        render(state);
      }
      else
      {
        printf("ERROR: Failed to launch game. Game's exe_path is NULL\n");
      }
    }
    else
    {
      printf("ERROR: Failed to start game. game_entries == NULL\n");
    }
  }
}

void move_select_up(State* state)
{
  if (state->selection.y > 0) state->selection.y--;
}

void move_select_down(State* state)
{
  if (state->selection.y < state->rows - 1)
  {
    if (get_real_selection_index(state) + state->columns >= state->game_entries_len)
    {
      // If there is a row below the current one
      if ((state->game_entries_len - (state->rows * state->columns * state->page)) / state->columns > state->selection.y)
      {
        int entries_on_page = state->game_entries_len - (state->rows * state->columns * state->page);

        state->selection.y = entries_on_page / state->columns;
        state->selection.x = entries_on_page - (state->columns * (entries_on_page / state->columns)) - 1;
      }
    }
    else
    {
      state->selection.y++;
    }
  }
}

void move_select_left(State* state)
{
  if (state->page > 0 && state->selection.x == 0)
  {
    state->page--;
    state->selection.x = state->columns - 1;
  }
  else if (state->selection.x > 0)
  {
    state->selection.x--;
  }
}

void move_select_right(State* state)
{
  if (state->page < state->game_entries_len / (state->rows * state->columns) && state->selection.x == state->columns - 1)
  {
    state->page++;
    state->selection.x = 0;
    
    if ((state->game_entries_len - (state->rows * state->columns * state->page)) / state->columns <= state->selection.y)
    {
      state->selection.y = (state->game_entries_len - (state->rows * state->columns * state->page)) / state->columns;
    }
  }
  else if (state->selection.x < state->columns - 1)
  {
    if (get_real_selection_index(state) < state->game_entries_len - 1)
    {
      state->selection.x++;
    }
  }
}

int get_real_selection_index(State* state)
{
  return (state->columns * state->selection.y) + (state->rows * state->columns) * state->page + state->selection.x;
}

void generate_new_game_name(State* state)
{
  if (state->game_entries != NULL && state->game_entries_len > 0)
  {
    // if (game_name_surface != NULL) SDL_FreeSurface(game_name_surface);
    SDL_Surface* game_name_surface = TTF_RenderUTF8_Solid(font_big, state->game_entries[get_real_selection_index(state)].game_title, TEXT_COLOR);
    if (game_name_surface == NULL) printf("%s\n", TTF_GetError());
    
    if (game_name_texture != NULL) SDL_DestroyTexture(game_name_texture);
    game_name_texture = SDL_CreateTextureFromSurface(state->renderer, game_name_surface);
    SDL_FreeSurface(game_name_surface);
    if (game_name_texture == NULL) printf("%s\n", SDL_GetError());
  }
}
