/*
  Fuzzy English +
  Inspired by Fuzzy Time and Fuzzy Time +
  With Date, 24H display and Week #
  New version by Bryan Denny
  http://www.bryandenny.com
  http://www.mandaria.net
  English adaptation by HBTeicher
  http://thepebbleblog.blogspot.dk/
  
  BT and Vibe fxns added by:
  Aaron Clymer aaron.clymer@yahoo.com
  14/09/2014
 */

#include <pebble.h>
#include "english_time.h"

#define ANIMATION_DURATION 800
#define LINE_BUFFER_SIZE 50
#define WINDOW_NAME "fuzzy_english_plus"

Window *window;
InverterLayer *inverter_layer;
bool bt_connect_toggle;
bool bar_hide_toggle;

typedef struct {
  TextLayer 		*layer[2];
} TextLine;

typedef struct {
  char 	line1[LINE_BUFFER_SIZE];
  char 	line2[LINE_BUFFER_SIZE];
  char 	line3[LINE_BUFFER_SIZE];
  char 	topbar[LINE_BUFFER_SIZE];
  char 	bottombar[LINE_BUFFER_SIZE];
} TheTime;

TextLine 	line1;
TextLine 	line2;
TextLine 	line3;
TextLine 	topbar;
TextLine 	bottombar;

static 	TheTime 	cur_time;
static 	TheTime 	new_time;

static 	bool 	busy_animating_in 	= false;
static 	bool 	busy_animating_out 	= false;
const 	int 	line1_y 						= 18;
const 	int 	line2_y 						= 56;
const 	int 	line3_y 						= 94;


void animationInStoppedHandler(struct Animation *animation, bool finished, void *context) {
  busy_animating_in = false;
	
  // reset cur_time
  cur_time = new_time;
}

void animationOutStoppedHandler(struct Animation *animation, bool finished, void *context) {
	
  // reset out layer to x=144
  TextLayer *outside = (TextLayer *)context;
  GRect rect = layer_get_frame(text_layer_get_layer(outside));
  if (rect.origin.y == line2_y) rect.origin.x = -144;
  else rect.origin.x = 144;
  layer_set_frame(text_layer_get_layer(outside), rect);

  busy_animating_out = false;
}

void updateLayer(TextLine *animating_line, int line) {

  TextLayer *inside, *outside;
  GRect rect = layer_get_frame(text_layer_get_layer(animating_line->layer[0]));

  inside = (rect.origin.x == 0) ? animating_line->layer[0] : animating_line->layer[1];
  outside = (inside == animating_line->layer[0]) ? animating_line->layer[1] : animating_line->layer[0];

  GRect in_rect = layer_get_frame(text_layer_get_layer(outside));
  GRect out_rect = layer_get_frame(text_layer_get_layer(inside));

  if (line == 2) {
    in_rect.origin.x += 144;
    out_rect.origin.x += 144;
  } else {
    in_rect.origin.x -= 144;
    out_rect.origin.x -= 144;
  }

 // animate out current layer
  busy_animating_out = true;
  PropertyAnimation *animate_out;
  animate_out = property_animation_create_layer_frame(text_layer_get_layer(inside), NULL, &out_rect);
  animation_set_duration((Animation*)animate_out, ANIMATION_DURATION);
  animation_set_curve((Animation*)animate_out, AnimationCurveEaseOut);
  animation_set_handlers((Animation*)animate_out, (AnimationHandlers) {
    .stopped = (AnimationStoppedHandler)animationOutStoppedHandler
  }, (void *)inside);
  animation_schedule((Animation*)animate_out);

  if (line==1){
    text_layer_set_text(outside, new_time.line1);
    text_layer_set_text(inside, cur_time.line1);
  }
  if (line==2){
    text_layer_set_text(outside, new_time.line2);
    text_layer_set_text(inside, cur_time.line2);
  }
  if (line==3){
    text_layer_set_text(outside, new_time.line3);
    text_layer_set_text(inside, cur_time.line3);
  }

  // animate in new layer
  busy_animating_in = true;
  PropertyAnimation *animate_in;
  animate_in = property_animation_create_layer_frame(text_layer_get_layer(outside), NULL, &in_rect);
  animation_set_duration((Animation*)animate_in, ANIMATION_DURATION);
  animation_set_curve((Animation*)animate_in, AnimationCurveEaseOut);
  animation_set_handlers((Animation*)animate_in, (AnimationHandlers) {
    .stopped = (AnimationStoppedHandler)animationInStoppedHandler
  }, (void *)outside);
  animation_schedule((Animation*)animate_in);
}

void update_watch(struct tm* t) {
	
  // Let's get the new text date
  info_lines(t, new_time.topbar, new_time.bottombar);

  // Let's update the top bar
  if(strcmp(new_time.topbar, cur_time.topbar) != 0) text_layer_set_text(topbar.layer[0], new_time.topbar);
	
  // Let's update the bottom bar
  text_layer_set_text(bottombar.layer[0], new_time.bottombar);

  // Let's get the new text time
  fuzzy_time(t, new_time.line1, new_time.line2, new_time.line3);

  // update hour only if changed
  if(strcmp(new_time.line1, cur_time.line1) != 0) updateLayer(&line1, 1);
	
  // update min1 only if changed
  if(strcmp(new_time.line2, cur_time.line2) != 0) updateLayer(&line2, 2);
	
  // update min2 only if changed happens on
  if(strcmp(new_time.line3, cur_time.line3) != 0) updateLayer(&line3, 3);

  // vibrate at o'clock from 8 to 20
  if(t->tm_min == 0 && t->tm_sec == 0 && t->tm_hour >= 8 && t->tm_hour <= 20 ) vibes_double_pulse();
  if(t->tm_min == 59 && t->tm_sec == 57 && t->tm_hour >= 7 && t->tm_hour <= 19 ) vibes_short_pulse();
}

// handle BT status change related events
void bluetooth_connection_handler(bool connected) {
	
	// Vibe pattern: (ON, OFF, ON, OFF, ON)
	static const uint32_t const segments[] = { 200, 100, 200, 100, 200 };
	VibePattern pat = {
  	.durations = segments,
  	.num_segments = ARRAY_LENGTH(segments),
	};
	
	// Long Vibe on connection status change
	if (!bt_connect_toggle && connected) {
		vibes_short_pulse();
		bt_connect_toggle = true;
	}
	if (bt_connect_toggle && !connected) {
		vibes_enqueue_custom_pattern(pat);
		bt_connect_toggle = false;
	}
		
	// Invert top/bottom bars on conection status change
	layer_set_hidden((Layer*) inverter_layer, bt_connect_toggle);
}

// Hide top/bottom bar with wrist flick 
void wrist_flick_handler(AccelAxisType axis, int32_t direction) {
	
	// switch the toggle
	bar_hide_toggle = !bar_hide_toggle;
	
	// hide/show the bars
	if (axis == 1) {
		layer_set_hidden(text_layer_get_layer(topbar.layer[0]), bar_hide_toggle);
		layer_set_hidden(text_layer_get_layer(bottombar.layer[0]), bar_hide_toggle);
	}
}

// Handle the start-up of the app
void handle_init(void) {
	
  // Create our app's base window
  window = window_create();
  window_stack_push(window, true);
  window_set_background_color(window, GColorBlack);

  // Init the text layers used to show the time

  // line1
  line1.layer[0] = text_layer_create(GRect(0, line1_y, 144, 50));
  text_layer_set_text_color(line1.layer[0], GColorWhite);
  text_layer_set_background_color(line1.layer[0], GColorClear);
  text_layer_set_font(line1.layer[0], fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
  text_layer_set_text_alignment(line1.layer[0], GTextAlignmentCenter);

  line1.layer[1] = text_layer_create(GRect(144, line1_y, 144, 50));
  text_layer_set_text_color(line1.layer[1], GColorWhite);
  text_layer_set_background_color(line1.layer[1], GColorClear);
  text_layer_set_font(line1.layer[1], fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
  text_layer_set_text_alignment(line1.layer[1], GTextAlignmentCenter);

  // line2
  line2.layer[0] = text_layer_create(GRect(0, line2_y, 144, 50));
  text_layer_set_text_color(line2.layer[0], GColorWhite);
  text_layer_set_background_color(line2.layer[0], GColorBlack);
  text_layer_set_font(line2.layer[0], fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
  text_layer_set_text_alignment(line2.layer[0], GTextAlignmentCenter);

  line2.layer[1] = text_layer_create(GRect(-144, line2_y, 144, 50));
  text_layer_set_text_color(line2.layer[1], GColorWhite);
  text_layer_set_background_color(line2.layer[1], GColorBlack);
  text_layer_set_font(line2.layer[1], fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
  text_layer_set_text_alignment(line2.layer[1], GTextAlignmentCenter);

  // line3
  line3.layer[0] = text_layer_create(GRect(0, line3_y, 144, 50));
  text_layer_set_text_color(line3.layer[0], GColorWhite);
  text_layer_set_background_color(line3.layer[0], GColorClear);
  text_layer_set_font(line3.layer[0], fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(line3.layer[0], GTextAlignmentCenter);

  line3.layer[1] = text_layer_create(GRect(144, line3_y, 144, 50));
  text_layer_set_text_color(line3.layer[1], GColorWhite);
  text_layer_set_background_color(line3.layer[1], GColorClear);
  text_layer_set_font(line3.layer[1], fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(line3.layer[1], GTextAlignmentCenter);

  // top text
  topbar.layer[0] = text_layer_create(GRect(0, 0, 144, 30));
  text_layer_set_text_color(topbar.layer[0], GColorWhite);
  text_layer_set_background_color(topbar.layer[0], GColorBlack);
  text_layer_set_font(topbar.layer[0], fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(topbar.layer[0], GTextAlignmentCenter);
	
  // bottom text
  bottombar.layer[0] = text_layer_create(GRect(0, 145, 144, 30));
  text_layer_set_text_color(bottombar.layer[0], GColorWhite);
  text_layer_set_background_color(bottombar.layer[0], GColorBlack);
  text_layer_set_font(bottombar.layer[0], fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(bottombar.layer[0], GTextAlignmentCenter);
	
	// inverter layer
	inverter_layer = inverter_layer_create(GRect(0, 0, 144, 168));
	
  // Ensures time is displayed immediately (will break if NULL tick event accessed).
  // (This is why it's a good idea to have a separate routine to do the update itself.)

  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  update_watch(t);
	
  Layer *root_layer = window_get_root_layer(window);
	
  layer_add_child(root_layer, text_layer_get_layer(line3.layer[0]));
  layer_add_child(root_layer, text_layer_get_layer(line3.layer[1]));
  layer_add_child(root_layer, text_layer_get_layer(line2.layer[0]));
  layer_add_child(root_layer, text_layer_get_layer(line2.layer[1]));
  layer_add_child(root_layer, text_layer_get_layer(line1.layer[0]));
  layer_add_child(root_layer, text_layer_get_layer(line1.layer[1]));
  layer_add_child(root_layer, text_layer_get_layer(bottombar.layer[0]));
  layer_add_child(root_layer, text_layer_get_layer(topbar.layer[0]));
  layer_add_child(root_layer, inverter_layer_get_layer(inverter_layer));
	
	// Subscribe to bluetooth service
	bluetooth_connection_service_subscribe(bluetooth_connection_handler);
	bt_connect_toggle = bluetooth_connection_service_peek();
	layer_set_hidden(inverter_layer_get_layer(inverter_layer), bt_connect_toggle);
	
	// Subscribe to accel service
	accel_tap_service_subscribe(wrist_flick_handler);
}

// Called once per second
void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {

  if (busy_animating_out || busy_animating_in) return;

  update_watch(tick_time);
}

// The main event/run loop for our app
int main(void) {
	handle_init();
	tick_timer_service_subscribe(MINUTE_UNIT, &handle_minute_tick);
  	app_event_loop();
  	//handle_deinit();
}