#include <pebble.h>

Window* window;
TextLayer *text_layer;
static GBitmap *s_bitmap;
static BitmapLayer *s_bitmap_layer;

//Default board
static char board_map[] = "1  2  3\n\n4  5  6\n\n7  8  9\nChoice:1\nStatus: New Game";

int choice = 1; 
int player = 1; //Player 1 is X, Player 2 is O
int winner = 0; //Flag to see if someone won, contains the player number for the winner, also used to check if reset function should be activated.
int count = 0; //Counter to check if all 9 slots have been used to calculate a draw

//Positions for the board
char board[3][3] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'}};

char status[10] = "New Game"; //Message to display the status on whos turn it is and who won

//Updates the display with the new values
void update_window() {
  snprintf(board_map, sizeof(board_map), "%c  %c  %c\n\n%c  %c  %c\n\n%c  %c  %c\nChoice:%d\nStatus: %s", board[0][0], board[0][1], board[0][2], board[1][0], board[1][1], board[1][2], board[2][0], board[2][1], board[2][2], choice, status);
  text_layer_set_text(text_layer, board_map);
}

//Resets all the values to their default to begin a new game
void reset() {
  snprintf(status,9*sizeof(char),"New Game");
  choice = 1;
  player = 0;
  winner = 0;
  count = 0;
  board[0][0] = '1';
  board[0][1] = '2';
  board[0][2] = '3'; 
  board[1][0] = '4'; 
  board[1][1] = '5'; 
  board[1][2] = '6'; 
  board[2][0] = '7'; 
  board[2][1] = '8'; 
  board[2][2] = '9';
  update_window();
}

//Updates the game with the new values and calculates if someone won or if the move is valid
void update_game() {
  int row;
  int column;
  int line;
  int option = choice;
  
  //Calculate whos turn it is
  player = count%2 + 1;
  
  //Calculate the coordinate based off the choice number
  row = --option/3;
  column = option%3;
  
  //Displays whos turn it is, has to be inverted to display the correct player in the UI
  snprintf(status,8*sizeof(char),"%c Turn", (player == 1) ? 'O' : 'X');
  
  //Check if space is empty
  if(board[row][column] != 'X' && board[row][column] != 'O') {
    board[row][column] = (player == 1) ? 'X' : 'O';

    //Check if someone won and set the play to the variable winner
    if((board[0][0] == board[1][1] && board[0][0] == board[2][2]) || (board[0][2] == board[1][1] && board[0][2] == board[2][0]))
      winner = player;
    else
      for(line = 0; line <= 2; line ++)
        if((board[line][0] == board[line][1] && board[line][0] == board[line][2]) || (board[0][line] == board[1][line] && board[0][line] == board[2][line]))
          winner = player;
  
    //Who won?
    if(winner == 0 && count >= 8) {
      snprintf(status,8*sizeof(char),"Draw");
      winner = -1;
      vibes_short_pulse();
    } else if (winner != 0) {
      snprintf(status,8*sizeof(char),"%c Wins!",(winner == 1) ? 'X' : 'O');
      vibes_short_pulse();
    } 
    
    //Increment the amount of moves have been played to see if its a draw
    count = count + 1;
    update_window();
  } else {
    
    //Space is not empty
    snprintf(status,8*sizeof(char),"Taken");
    vibes_short_pulse();
    update_window();
  }
}

//Logic for pressing the up button
void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  //If theres been a winner reset the game
  if(winner != 0) {
    reset();
  } else {
    //If there has not been a winner increment choice by one and circle back to 1 if you go passed 9
    choice = choice + 1;
    if(choice>9)
      choice = choice - 9;
    update_window();
  }
}
 
//Logic for pressing the down button
void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  //If theres been a winner reset the game
  if(winner != 0) {
    reset();
  } else {
    //If there has not been a winner decrement choice by one and circle back to 9 if you go lower than 1
    choice = choice - 1;
    if(choice<1)
      choice = choice + 9;
    update_window();
  }
}
 
//Logic for pressing the select button
void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  //If theres been a winner reset the game
  if(winner != 0) {
    reset();
  } else { 
    //If there has not been a winner update the game thus placing a move
    update_game();
  }
}

//Links buttons to their respective logic
void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

/* Load all Window sub-elements */
void window_load(Window *window) {
  //Create variable that holds the window information for easier access in functions
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  //Create text layer with 140 pixel width and 168 height
  text_layer = text_layer_create(GRect(0, 0, 140, 168));
  text_layer_set_background_color(text_layer, GColorClear);
  text_layer_set_text_color(text_layer, GColorBlack);
  
  //Sets the font to Gothic Bold with a font size of 24 and a center alignment
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  
  //Create the image layer used by the grid
  s_bitmap_layer = bitmap_layer_create(bounds);
  s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_grid);
  bitmap_layer_set_compositing_mode(s_bitmap_layer, GCompOpAssign);
  bitmap_layer_set_bitmap(s_bitmap_layer, s_bitmap);
  bitmap_layer_set_alignment(s_bitmap_layer, GAlignCenter);
  
  //Applies the Grid image to the window then applies the text layer that hold the choices on top of it
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bitmap_layer));
  layer_add_child(bitmap_layer_get_layer(s_bitmap_layer), (Layer*) text_layer);  
  
  //Adds the default values to the text layer so it is populated on launch of the application
  text_layer_set_text(text_layer, board_map);
}
 
// Removes The image and text layers from the window 
void window_unload(Window *window) {
  text_layer_destroy(text_layer);
  gbitmap_destroy(s_bitmap);
  bitmap_layer_destroy(s_bitmap_layer);
}
 
// Initialize everything, creates the window that holds the text and image layers
void init() {
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  WindowHandlers handlers = {
    .load = window_load,
    .unload = window_unload
  };
  window_set_window_handlers(window, (WindowHandlers) handlers);
  window_stack_push(window, true);
}
 
// Destroys the window 
void deinit() {
  window_destroy(window);
}
 
// Main app lifecycle 
int main(void) {
  //Initializes everything when the application is launched
  init();
  app_event_loop();
  
  //Destroys everything when the application is closed
  deinit();
}