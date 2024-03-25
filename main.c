#include "main.h"

int main(int argc, char* argv[])
{
  printf("Starting Adelfors Arcade...\n");

  State state = { .rows = 2, .columns = 4};

  printf("Initializing SDL...\n");
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK))
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

  SDL_GetRendererOutputSize(state.renderer, &state.window_w, &state.window_h);

  SDL_SysWMinfo wm_info;
  SDL_VERSION(&wm_info.version);
  SDL_GetWindowWMInfo(state.window, &wm_info);
  state.window_handle = wm_info.info.win.window;

  SetFocus(state.window_handle);
  SetCursorPos(state.window_w, 0);
  SDL_ShowCursor(SDL_DISABLE);
  // ShowCursor(0);
  
  if (load_font(&state, "./font/font.ttf") == 1) return EXIT_FAILURE;

  set_menu_state(&state, LOADING);

  if (load_arcade_images(&state) == 1) return EXIT_FAILURE;

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

  set_menu_state(&state, GAME_SELECT);
  
  while (should_quit == 0)
  {
    if (state.menu_state != LOADING && state.game_process_handle != NULL)
    {
      CloseHandle(state.game_process_handle);
      state.game_process_handle = NULL;
      if (state.window_handle != NULL) SetFocus(state.window_handle);
      else printf("ERROR: Window handle null. Can't set focus\n");
    }
    
    should_quit = handle_events(&state);

    // Limit loop frequency to leave more resources for the actual games and to save on electricity
    SDL_Delay(TICK_RATE);
  }

  if (game_name_texture != NULL) SDL_DestroyTexture(game_name_texture);
  if (run_game_hint_texture != NULL) SDL_DestroyTexture(run_game_hint_texture);
  if (loading_text_texture != NULL) SDL_DestroyTexture(loading_text_texture);
  if (no_games_text_texure != NULL) SDL_DestroyTexture(no_games_text_texure);
  if (button_accept_texture != NULL) SDL_DestroyTexture(button_accept_texture);
  if (page_text_texture != NULL) SDL_DestroyTexture(page_text_texture);
  if (arrow_texture != NULL) SDL_DestroyTexture(arrow_texture);
  
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
              if (state->menu_state == GAME_SELECT) move_select_right(state);
            }
            else if (event.jaxis.value < 0)
            {
              if (state->menu_state == GAME_SELECT) move_select_left(state);
            }
          }
          
          // Up-down
          if (event.jaxis.axis == 1)
          {
            if (event.jaxis.value > 0)
            {
              if (state->menu_state == GAME_SELECT) move_select_down(state);
            }
            else if (event.jaxis.value < 0)
            {
              if (state->menu_state == GAME_SELECT) move_select_up(state);
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
            if (state->menu_state == GAME_SELECT)
            {
              // Start game!
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
            if (state->menu_state == GAME_SELECT)
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
            if (state->menu_state == GAME_SELECT)
            {
              move_select_up(state);
            }
            break;
          }
          case SDL_SCANCODE_DOWN:
          {
            if (state->menu_state == GAME_SELECT)
            {
              move_select_down(state);
            }
            break;
          }
          case SDL_SCANCODE_LEFT:
          {
            if (state->menu_state == GAME_SELECT)
            {
              move_select_left(state);
            }
            break;
          }
          case SDL_SCANCODE_RIGHT:
          {
            if (state->menu_state == GAME_SELECT)
            {
              move_select_right(state);
            }
            break;
          }

          case SDL_SCANCODE_RETURN:
          {
            if (state->menu_state == GAME_SELECT)
            {
              // Start game!
              if (state->game_process_handle == NULL) state->game_process_handle = CreateThread(NULL, 0, start_game_thread, state, 0, NULL);
              else printf("ERROR: Failed to start game. game_process_handle is not null\n");
            }
            else if (state->menu_state == SPLASH)
            {
              set_menu_state(state, GAME_SELECT);
            }

            break;
          }

          case SDL_SCANCODE_G:
          {
            if (state->menu_state != GAME_SELECT) break;
            
            // If Left Shift and R are held down when G is pressed
            if (keyboard_state[SDL_SCANCODE_LSHIFT] == 1 && keyboard_state[SDL_SCANCODE_R] == 1)
            {
              set_menu_state(state, LOADING);
              render(state);
              
              printf("Searching for games...\n");
              find_games(state);
              printf("Search done\n");

              set_menu_state(state, GAME_SELECT);
            }

            break;
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

  if (state->menu_state == LOADING) render_loading_ui(state);
  else if (state->menu_state == GAME_SELECT) render_game_select_ui(state);
  else if (state->menu_state == SPLASH) render_splash_ui(state);
  else if (state->menu_state == SHUTDOWN) render_shutdown_ui(state);

  SDL_RenderPresent(state->renderer);
}

void render_loading_ui(State* state)
{
  if (state == NULL)
  {
    printf("ERROR: render_loading_ui: state is null");
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
      
        SDL_Rect rect = {
          left + (w / state->columns) * column,
          top + (h / state->rows) * row,
          w / state->columns,
          h / state->rows,
        };
        SDL_RenderFillRect(state->renderer, &rect);

        if (state->game_entries[count + ((state->rows * state->columns) * state->page)].game_image != NULL)
        {
          SDL_Texture* image_texture = state->game_entries[count + ((state->rows * state->columns) * state->page)].game_image;
          SDL_Rect image_rect = {0, 0, 0, 0};
          SDL_QueryTexture(image_texture, NULL, NULL, &image_rect.w, &image_rect.h);

          SDL_Rect pos_rect = {
            left + (w / state->columns) * column + ARCADE_GAME_BOX_PADDING,
            top + (h / state->rows) * row + ARCADE_GAME_BOX_PADDING,
            w / state->columns - ARCADE_GAME_BOX_PADDING * 2,
            h / state->rows - ARCADE_GAME_BOX_PADDING * 2
          };

          SDL_RenderCopy(state->renderer, image_texture, NULL, &pos_rect);
        }

        count++;
      }
    }
    
    if (button_accept_texture != NULL)
    {
      SDL_Rect btn_rect = {0, 0, 0, 0};
      SDL_QueryTexture(button_accept_texture, NULL, NULL, &btn_rect.w, &btn_rect.h);

      SDL_Rect pos_rect = {state->window_w - 500, state->window_h - 100, 75, 75};

      SDL_RenderCopy(state->renderer, button_accept_texture, NULL, &pos_rect);
    }

    if (run_game_hint_texture != NULL)
    {
      SDL_Rect text_rect = {0, 0, 0, 0};
      SDL_QueryTexture(run_game_hint_texture, NULL, NULL, &text_rect.w, &text_rect.h);
      text_rect.x = state->window_w - text_rect.w - 50;
      text_rect.y = state->window_h - 75;

      SDL_RenderCopy(state->renderer, run_game_hint_texture, NULL, &text_rect);
    }

    if (state->page > 0)
    {
      if (arrow_texture != NULL)
      {
        SDL_Rect arrow_left_rect = {0, 0, 0, 0};
        SDL_QueryTexture(arrow_texture, NULL, NULL, &arrow_left_rect.w, &arrow_left_rect.h);

        int pos_w = 100;
        int pos_h = pos_w * 1.5;
        SDL_Rect pos_rect = {left - 50 - pos_w, bottom - (h / 2) - (pos_h / 2), pos_w, pos_h};

        SDL_RendererFlip flip = SDL_FLIP_HORIZONTAL;

        SDL_RenderCopyEx(state->renderer, arrow_texture, NULL, &pos_rect, 0, NULL, flip);
      }
    }

    if (state->game_entries_len / (state->rows * state->columns) > state->page)
    {
      if (arrow_texture != NULL)
      {
        SDL_Rect arrow_right_rect = {0, 0, 0, 0};
        SDL_QueryTexture(arrow_texture, NULL, NULL, &arrow_right_rect.w, &arrow_right_rect.h);

        int pos_w = 100;
        int pos_h = pos_w * 1.5;
        SDL_Rect pos_rect = {left + w + 50, bottom - (h / 2) - (pos_h / 2), pos_w, pos_h};

        SDL_RenderCopy(state->renderer, arrow_texture, NULL, &pos_rect);
      }
    }

    if (state->game_entries_len > state->rows * state->columns && page_text_texture != NULL)
    {
      SDL_Rect text_rect = {0, 0, 0, 0};
      SDL_QueryTexture(page_text_texture, NULL, NULL, &text_rect.w, &text_rect.h);
      text_rect.x = state->window_w / 2 - text_rect.w / 2;
      text_rect.y = bottom + ((state->window_h - bottom) / 2) - text_rect.h / 2;

      SDL_RenderCopy(state->renderer, page_text_texture, NULL, &text_rect);
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

void render_splash_ui(State* state)
{
  if (version_number_texture != NULL)
  {
    SDL_Rect text_rect = {0, 0, 0, 0};
    SDL_QueryTexture(version_number_texture, NULL, NULL, &text_rect.w, &text_rect.h);
    text_rect.x = state->window_w - text_rect.w - 25;
    text_rect.y = state->window_h - text_rect.h - 25;

    SDL_RenderCopy(state->renderer, version_number_texture, NULL, &text_rect);
  }
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

    SDL_Surface* run_game_hint_surface = TTF_RenderUTF8_Solid(font_medium, "Starta spel", TEXT_COLOR_FADED);
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

    SDL_Surface* loading_text_surface = TTF_RenderUTF8_Solid(font_big, "LADDAR...", TEXT_COLOR);
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

    SDL_Surface* no_games_text_surface = TTF_RenderUTF8_Solid(font_big, "Inga spel hittades", TEXT_COLOR);
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
    
    SDL_Surface* version_number_surface = TTF_RenderUTF8_Solid(font_medium, VERSION_STRING_LITERAL, TEXT_COLOR_FADED);
    if (version_number_surface == NULL)
    {
      printf("ERROR: Failed to create surface for version number text: %s\n", TTF_GetError());
      return 1;
    }
    else
    {
      if (version_number_texture != NULL) SDL_DestroyTexture(version_number_texture);
      version_number_texture = SDL_CreateTextureFromSurface(state->renderer, version_number_surface);
      SDL_FreeSurface(version_number_surface);
      if (version_number_texture == NULL)
      {
        printf("ERROR: Failed to create texture for version number text: %s\n", SDL_GetError());
        return 1;
      }
    }
  }

  return 0;
}

int load_arcade_images(State* state)
{
  int failed = 0;
  
  printf("Loading arcade images...\n");
  
  SDL_Surface* button_accept_surface = IMG_Load("./images/arcade_button_green.png");
  
  if (button_accept_surface != NULL)
  {
    if ((button_accept_texture = SDL_CreateTextureFromSurface(state->renderer, button_accept_surface)) == NULL)
    {
      printf("  ERROR: Creating texture for arcade_button_green failed\n");
      failed = 1;
    }

    SDL_FreeSurface(button_accept_surface);
  }
  else
  {
    printf("  ERROR: Failed to load \"arcade_button_green.png\"\n");
    failed = 1;
  }

  SDL_Surface* arrow_surface = IMG_Load("./images/arcade_arrow.png");

  if (arrow_surface != NULL)
  {
    if ((arrow_texture = SDL_CreateTextureFromSurface(state->renderer, arrow_surface)) == NULL)
    {
      printf("  ERROR: Creating texture for arrow failed\n");
      failed = 1;
    }

    SDL_FreeSurface(arrow_surface);
  }
  else
  {
    printf("  ERROR: Failed to load \"arcade_arrow.png\"\n");
    failed = 1;
  }

  return failed;
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
        if (state->game_entries[i].game_image != NULL)
        {
          SDL_DestroyTexture(state->game_entries[i].game_image);
          state->game_entries[i].game_image = NULL;
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
  if (state == NULL)
  {
    printf("ERROR: find_games(): state is null\n");
    return 1;
  }
  
  const char GAMES_PATH[] = "./games";
  
  DIR* dir = opendir(GAMES_PATH);

  if (dir != NULL)
  {
    free_game_entries(state);
    state->selection = (Position){0, 0};
    state->page = 0;
    
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

      int path_len = strlen(path);
        
      struct stat s;
      if (stat(path, &s) == 0)
      {
        // If what we found is a directory
        if (s.st_mode & S_IFDIR)
        {
          // Open the directory and look for a .exe
          DIR* game_dir = opendir(path);
          
          if (game_dir != NULL)
          {
            char exe_found = 0;
            char* icon_name = NULL;
            
            struct dirent* dirp2;
            while((dirp2 = readdir(game_dir)) != NULL)
            {
              if (strcmp(dirp2->d_name, ".") == 0 || strcmp(dirp2->d_name, "..") == 0 || strcmp(dirp2->d_name, "desktop.ini") == 0) continue;
              // Ignore some potential .exes
              if (strcmp(dirp->d_name, "UnityCrashHandler64.exe\0") == 0) continue;
              
              int name_len = strlen(dirp2->d_name);
              if (name_len > 4)
              {
                if (dirp2->d_name[name_len - 4] == '.' && dirp2->d_name[name_len - 3] == 'e' && dirp2->d_name[name_len - 2] == 'x' && dirp2->d_name[name_len - 1] == 'e' && exe_found == 0)
                {
                  printf("  [%s]\n", dirp->d_name);

                  state->game_entries_len++;

                  state->game_entries = (GameEntry*)realloc(state->game_entries, sizeof(GameEntry) * state->game_entries_len);
                  if (state->game_entries == NULL)
                  {
                    printf("  ERROR: Failed to reallocate memory for game_entries\n");
                    break;
                  }
                  state->game_entries[state->game_entries_len - 1].game_image = NULL;
                  state->game_entries[state->game_entries_len - 1].game_title = malloc(dirp_name_len + 1);
                  strcpy(state->game_entries[state->game_entries_len - 1].game_title, dirp->d_name);

                  char* exe_path = malloc(path_len + name_len + 3);
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
                  exe_found = 1;

                  printf("    %s\n", exe_path);

                  if (icon_name != NULL)
                  {
                    char* icon_path = malloc(path_len + strlen(icon_name) + 1);
                    sprintf(icon_path, "%s/%s", path, icon_name);

                    SDL_Surface* image_surface = IMG_Load(icon_path);
                    SDL_Texture* image_texture;

                    if (image_surface != NULL)
                    {
                      image_texture = SDL_CreateTextureFromSurface(state->renderer, image_surface);
                      if (image_texture != NULL)
                      {
                        state->game_entries[state->game_entries_len - 1].game_image = image_texture;
                        printf("    Icon found\n");
                      }

                      SDL_FreeSurface(image_surface);
                    }

                    free(icon_name);
                    icon_name = NULL;
                    free(icon_path);
                    icon_path = NULL;
                  }
                  
                  continue;
                }
                else if (strcmp(dirp2->d_name, "game_icon.png\0") == 0 || strcmp(dirp2->d_name, "game_icon.jpg\0") == 0)
                {
                  icon_name = malloc(14);
                  strcpy(icon_name, dirp2->d_name);
                  
                  if (exe_found == 1)
                  {
                    char* icon_path = malloc(path_len + strlen(icon_name) + 1);
                    sprintf(icon_path, "%s/%s", path, icon_name);

                    SDL_Surface* image_surface = IMG_Load(icon_path);
                    SDL_Texture* image_texture;

                    if (image_surface != NULL)
                    {
                      image_texture = SDL_CreateTextureFromSurface(state->renderer, image_surface);
                      if (image_texture != NULL)
                      {
                        state->game_entries[state->game_entries_len - 1].game_image = image_texture;
                        printf("    Icon found\n");
                      }
                      else printf("  ERROR: image_texture null: %s\n", SDL_GetError());

                      SDL_FreeSurface(image_surface);
                    }
                    else printf("  ERROR: image_surface null: %s\n", SDL_GetError());
                    
                    free(icon_name);
                    icon_name = NULL;
                    free(icon_path);
                    icon_path = NULL;
                  }
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

    generate_page_text(state);
    
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
  if (state == NULL)
  {
    printf("Error: start_game_thread: state is null\n");
    return 0;
  }

  set_menu_state(state, LOADING);
  run_selected_game(state);
  set_menu_state(state, GAME_SELECT);
  
  return 0;
}

void run_selected_game(State* state)
{
  if (state->game_entries != NULL)
  {
    int sel_idx = get_real_selection_index(state);
    if (state->game_entries[sel_idx].exe_path != NULL)
    {
      printf("---Launching \"%s\"...---\n", state->game_entries[sel_idx].game_title);
        
      STARTUPINFO si;
      PROCESS_INFORMATION pi;

      si.dwX = 0;
      si.dwY = 0;
      si.dwXSize = state->window_w;
      si.dwYSize = state->window_h;
      si.dwFlags = STARTF_RUNFULLSCREEN;

      ZeroMemory(&si, sizeof(si));
      si.cb = sizeof(si);
      ZeroMemory(&pi, sizeof(pi));

      if (!CreateProcess(
        NULL,
        state->game_entries[sel_idx].exe_path,
        NULL,
        NULL,
        FALSE,
        HIGH_PRIORITY_CLASS | WS_POPUP,
        NULL,
        NULL,
        &si,
        &pi))
      {
        printf("ERROR: Failed to launch game. CreateProcess failed: %lu\n", GetLastError());
        return;
      }

      SetFocus(pi.hProcess);
      SetCursorPos(state->window_w, 0);

      WaitForSingleObject(pi.hProcess, INFINITE);
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);

      printf("\n---\"%s\" exited---\n", state->game_entries[sel_idx].game_title);
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
    generate_page_text(state);
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
    generate_page_text(state);
    
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
    SDL_Surface* game_name_surface = TTF_RenderUTF8_Solid(font_big, state->game_entries[get_real_selection_index(state)].game_title, TEXT_COLOR);
    if (game_name_surface == NULL) printf("ERROR: generate_new_game_name(): %s\n", TTF_GetError());
    
    if (game_name_texture != NULL) SDL_DestroyTexture(game_name_texture);
    game_name_texture = SDL_CreateTextureFromSurface(state->renderer, game_name_surface);
    if (game_name_texture == NULL) printf("ERROR: generate_new_game_name(): %s\n", SDL_GetError());
    SDL_FreeSurface(game_name_surface);
  }
}

void generate_page_text(State* state)
{
  const int len = 2 + ((state->page / 10) + 1) + (state->game_entries_len / (state->rows * state->columns) + 1) + 1;
  char* str = calloc(len, sizeof(char));

  if (str != NULL)
  {
    sprintf(str, "[%d/%d]", state->page + 1, state->game_entries_len / (state->rows * state->columns) + 1);
  }
  
  SDL_Surface* page_text_surface = TTF_RenderUTF8_Solid(font_big, str, TEXT_COLOR_FADED);
  if (page_text_surface == NULL) printf("ERROR: generate_page_text(): %s\n", TTF_GetError());

  if (page_text_texture != NULL) SDL_DestroyTexture(page_text_texture);
  page_text_texture = SDL_CreateTextureFromSurface(state->renderer, page_text_surface);
  if (page_text_texture == NULL) printf("ERROR: generate_page_text(): %s\n", SDL_GetError());
  SDL_FreeSurface(page_text_surface);

  if (str != NULL) free(str);
}

void set_menu_state(State* state, MenuState new_state)
{
  switch (new_state)
  {
    case LOADING:
    {
      if (state != NULL)
      {
        state->menu_state = new_state;
        state->input_enabled = 0;
        render(state);
      }

      break;
    }
    case GAME_SELECT:
    {
      if (state != NULL)
      {
        state->menu_state = new_state;
        state->input_enabled = 1;
        render(state);
      }

      break;
    }
    case SPLASH:
    {
      if (state != NULL)
      {
        state->menu_state = new_state;
        state->input_enabled = 1;
        render(state);
      }
    }
    default:
    {
      break;
    }
  }
}
