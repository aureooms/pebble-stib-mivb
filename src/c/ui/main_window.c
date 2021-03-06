#include <pebble.h>
#include "main_window.h"
#include "color.h"
#include "draw.h"
#include "click.h"
#include "../data/stops.h"
#include "../ds/dynamicarray.h"
#include "../pebble/translate_error.h"

// initialized once, deleted on app kill
Window *ui_main_window = NULL;
// initialized once, deleted on app kill
StatusBarLayer *ui_status_bar = NULL;
// initialized once, deleted on app kill
TextLayer *ui_info_layer = NULL;
// initialized once, deleted on app kill
TextLayer *ui_stop_name_layer = NULL;
// initialized once, deleted on app kill
TextLayer *ui_message_layer = NULL;

// message displayed when no information is available at the current stop
const char *MESSAGE_NOTHING = "nothing right now";

// used to remember which stop we are displaying
uint32_t ui_displayed_stop_id = 0;
size_t ui_displayed_stop_index = 0;

// memory necessary to display realtime
char ui_minutes_buffer[DISPLAYED_ITEMS][WHEN_BUFFER_SIZE];

// dynamically allocated and freed memory for layers displaying realtime
TextLayer *ui_line_number_layer[DISPLAYED_ITEMS] = { NULL };
TextLayer *ui_destination_name_layer[DISPLAYED_ITEMS] = { NULL };
TextLayer *ui_minutes_layer[DISPLAYED_ITEMS] = { NULL };

// to store the dimensions of the main window
static GRect s_rect;
static GSize s_size;
static int16_t s_width;
static int16_t s_height;
static int16_t s_left;
static int16_t s_top;
static int16_t s_title;

GRect get_main_window_rect(){
  return s_rect;
}

GSize get_main_window_size(){
  return s_size;
}

int16_t get_main_window_width(){
  return s_width;
}

int16_t get_main_window_height(){
  return s_height;
}

int16_t get_main_window_left(){
  return s_left;
}

int16_t get_main_window_top(){
  return s_top;
}

int16_t get_main_window_title(){
  return s_title;
}

void main_window_load(Window *window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "main_window_load");

  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  s_rect = layer_get_bounds(window_layer);

  s_size = s_rect.size ;
  s_width = s_size.w ;
  s_height = s_size.h ;
  s_left = MARGIN_LEFT;
  s_top = MARGIN_TOP;
  s_title = TITLE_HEIGHT;

  // Set background color
  window_set_background_color(window, BG);

  // Create the StatusBarLayer
  ui_status_bar = status_bar_layer_create();
  status_bar_layer_set_colors(ui_status_bar, BKO, FKO);
  status_bar_layer_set_separator_mode(ui_status_bar, StatusBarLayerSeparatorModeNone);

  // Stop name text layer
  ui_stop_name_layer = text_layer_create(GRect(s_left+10, s_top, s_width - 2*(s_left+10), s_title));
  text_layer_set_font(ui_stop_name_layer, fonts_get_system_font(FONT));
  text_layer_set_text_alignment(ui_stop_name_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(ui_stop_name_layer, GTextOverflowModeTrailingEllipsis);
  text_layer_set_text_color(ui_stop_name_layer, GColorBlack);

  // Message text layer
  ui_message_layer = text_layer_create(GRect(s_left, s_top+s_title, s_width - 2*s_left, s_height - s_top - s_title));
  text_layer_set_font(ui_message_layer, fonts_get_system_font(FONT));
  text_layer_set_text_alignment(ui_message_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(ui_message_layer, GTextOverflowModeWordWrap);
  text_layer_set_text_color(ui_message_layer, BKO);
  
  // Init data
  ds_DynamicArray_init(&data_stops_curr, 1);
  ds_DynamicArray_init(&data_stops_recv, 1);
  
  // Load cached data
  thaw();

  // Update display with cached information
  draw(true);

  // Register click events
  window_set_click_config_provider(window, (ClickConfigProvider) click_config_provider);

}

void main_window_unload(Window *window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "main_window_unload");
  clear();
  status_bar_layer_destroy(ui_status_bar);
  text_layer_destroy(ui_stop_name_layer);
  text_layer_destroy(ui_message_layer);
  
  freeze();
  Stops_clear(&data_stops_curr);
  Stops_clear(&data_stops_recv);
}

status_t freeze(){
  APP_LOG(APP_LOG_LEVEL_DEBUG, "freeze");
  persist_write_int(UI_DISPLAYED_STOP_ID_PERSIST_KEY, ui_displayed_stop_id);
  uint32_t key = STOPS_PERSIST_KEY_BEGIN;
  status_t status = Stops_persist_write(&key, &data_stops_curr);
  if (status > 0){
    APP_LOG(APP_LOG_LEVEL_DEBUG, "freeze > wrote %ld bytes", status);
  }
  else{
    APP_LOG(APP_LOG_LEVEL_ERROR, "freeze > catched %s", pebble_translate_status_error(status));
  }
  return status;
}

uint32_t thaw(){
  APP_LOG(APP_LOG_LEVEL_DEBUG, "thaw");
  
  ui_displayed_stop_id = persist_read_int(UI_DISPLAYED_STOP_ID_PERSIST_KEY); // defaults to zero
  
  uint32_t key = STOPS_PERSIST_KEY_BEGIN;
  const uint32_t end = Stops_persist_read(&key, &data_stops_curr);
  // seems to be slow and unnecessary
  // Stops_persist_clear(STOPS_PERSIST_KEY_BEGIN, end); // clean up
  
  update_displayed_stop_index();
  return end;
}

void update_displayed_stop_index(){
  ui_displayed_stop_index = 0; // default: show closest
  const size_t n = data_stops_curr.length;
  for (size_t i = 0 ; i < n ; ++i){
    Stop *stop = data_stops_curr.data[i];
    if (stop->id == ui_displayed_stop_id) {
      ui_displayed_stop_index = i;
      break;
    }
  }
}

void update_displayed_stop_id(){
  if (data_stops_curr.length > 0) {
    Stop *stop = data_stops_curr.data[ui_displayed_stop_index];
    ui_displayed_stop_id = stop->id;
  }
}