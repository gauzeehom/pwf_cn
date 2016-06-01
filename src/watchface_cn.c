#include <pebble.h>
#include "LunarSolarConverter.h"

static Window *s_main_window;
static TextLayer *s_time_layer, *s_date_layer, *s_lunar_layer, *s_connection_layer, *s_battery_layer;

static void battery_handler(BatteryChargeState charge_state) {
  static char battery_text[] = "100% charged";
  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "charging");
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
  }
  text_layer_set_text(s_battery_layer, battery_text);
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_time_buffer[8];
  strftime(s_time_buffer, sizeof(s_time_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_time_buffer);

  // Copy date into buffer from tm structure
  static char s_date_buffer[16];
  strftime(s_date_buffer, sizeof(s_date_buffer), "%F", tick_time);

  // Show the date
  text_layer_set_text(s_date_layer, s_date_buffer);

  // get lunar from tm structure
  const char * lunarStr = SolarToLunarString(tick_time->tm_year + 1900, tick_time->tm_mon + 1, tick_time->tm_mday, tick_time->tm_wday);

  // Show the date
  text_layer_set_text(s_lunar_layer, lunarStr);
  free((char*)lunarStr);

  battery_handler(battery_state_service_peek());
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void bluetooth_handler(bool connected) {
  text_layer_set_text(s_connection_layer, connected ? "pebble" : "no connection");
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create connection Layer
  s_connection_layer = text_layer_create(GRect(0, 0, bounds.size.w / 2, bounds.size.h * 2 / 12));
  text_layer_set_text_color(s_connection_layer, GColorClear);
  text_layer_set_background_color(s_connection_layer, GColorBlack);
  text_layer_set_font(s_connection_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_connection_layer, GTextAlignmentLeft);
  bluetooth_handler(bluetooth_connection_service_peek());
  // Add to Window
  layer_add_child(window_layer, text_layer_get_layer(s_connection_layer));

  // Create battery meter Layer
  s_battery_layer = text_layer_create(GRect(bounds.size.w / 2, 0, bounds.size.w / 2, bounds.size.h * 2 / 12));
  text_layer_set_text_color(s_battery_layer, GColorClear);
  text_layer_set_background_color(s_battery_layer, GColorBlack);
  text_layer_set_font(s_battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentRight);
  text_layer_set_text(s_battery_layer, "battery");
  // Add to Window
  layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));

  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(GRect(0, bounds.size.h * 2 / 12, bounds.size.w, bounds.size.h * 4 / 12));
  // Improve the layout to be more like a watchface
  text_layer_set_text_color(s_time_layer, GColorClear);
  text_layer_set_background_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  // Create date TextLayer
  s_date_layer = text_layer_create(GRect(0, bounds.size.h / 2, bounds.size.w, bounds.size.h / 4));
  text_layer_set_text_color(s_date_layer, GColorClear);
  text_layer_set_background_color(s_date_layer, GColorBlack);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  // Add to Window
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

  // Create lunar TextLayer
  s_lunar_layer = text_layer_create(GRect(0, bounds.size.h * 3 / 4, bounds.size.w, bounds.size.h / 4));
  text_layer_set_text_color(s_lunar_layer, GColorClear);
  text_layer_set_background_color(s_lunar_layer, GColorBlack);
  text_layer_set_font(s_lunar_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  text_layer_set_text_alignment(s_lunar_layer, GTextAlignmentCenter);
  // Add to Window
  layer_add_child(window_layer, text_layer_get_layer(s_lunar_layer));
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_lunar_layer);
  text_layer_destroy(s_connection_layer);
  text_layer_destroy(s_battery_layer);
}


static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Make sure the time is displayed from the start
  update_time();

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  battery_state_service_subscribe(battery_handler);

  bluetooth_connection_service_subscribe(bluetooth_handler);
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

