#include <pebble.h>
#include "main.h"

uint32_t key = 0;

static Window *window;
static Layer *s_background_layer, *s_date_layer, *s_hands_layer;
static TextLayer *s_weekday_label, *s_day_label, *s_month_label;
static char s_weekday_buffer[16], s_date_buffer[16], s_month_buffer[16];
static GPath *s_minute_arrow, *s_hour_arrow, *s_tick_paths[NUM_CLOCK_TICKS];

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////// Callbacks ///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

static void bluetooth_callback(bool connected) {
// 	char *select_bluetooth_disconnect = "";
// 	persist_read_string(MESSAGE_KEY_SELECT_BLUETOOTH_DISCONNECT,select_bluetooth_disconnect,5);
// 	int select_bluetooth_disconnect_int = atoi(select_bluetooth_disconnect);
	
	bool message = persist_read_bool(key);	
	
	int colour_background = persist_read_int(MESSAGE_KEY_COLOUR_BACKGROUND);
	int colour_weekday = persist_read_int(MESSAGE_KEY_COLOUR_WEEKDAY);
	int colour_month = persist_read_int(MESSAGE_KEY_COLOUR_MONTH);
	int colour_bluetooth = persist_read_int(MESSAGE_KEY_COLOUR_BLUETOOTH);
	
	if(!connected) {												
		if(message) {												// Disconected with config
			text_layer_set_text_color(s_weekday_label, PBL_IF_BW_ELSE(gcolor_legible_over(GColorFromHEX(colour_background)), GColorFromHEX(colour_bluetooth)));	// Set Top Colour
			text_layer_set_text_color(s_month_label, PBL_IF_BW_ELSE(gcolor_legible_over(GColorFromHEX(colour_background)), GColorFromHEX(colour_bluetooth)));		// Set Bottom Colour
		} else { 									// Disconnected and no config
			text_layer_set_text_color(s_weekday_label, PBL_IF_BW_ELSE(GColorBlack, GColorRed));	// Set Top Colour
			text_layer_set_text_color(s_month_label, PBL_IF_BW_ELSE(GColorBlack, GColorRed));	// Set Bottom Colour
		}
// 		if(select_bluetooth_disconnect_int == 0) { }		// No vibration 
// 		else if(select_bluetooth_disconnect_int == 1) { vibes_short_pulse(); }	// Short vibration
// 		else if(select_bluetooth_disconnect_int == 2) { vibes_long_pulse(); }	// Long vibration
// 		else if(select_bluetooth_disconnect_int == 3) { vibes_double_pulse(); }	// Double vibration
// 		else { 
		vibes_long_pulse();
// 		}	// Default
	} else {														// Connected
		if(message) {
			text_layer_set_text_color(s_weekday_label, PBL_IF_BW_ELSE(gcolor_legible_over(GColorFromHEX(colour_background)), GColorFromHEX(colour_weekday)));	// Set Top Colour
			text_layer_set_text_color(s_month_label, PBL_IF_BW_ELSE(gcolor_legible_over(GColorFromHEX(colour_background)), GColorFromHEX(colour_month)));		// Set Bottom Colour
		} else {	
			text_layer_set_text_color(s_weekday_label, GColorWhite);
			text_layer_set_text_color(s_month_label, GColorWhite);
		}
	}
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
	persist_write_bool(key, true);
// Colours
	Tuple *colour_background_t	= dict_find(iter, MESSAGE_KEY_COLOUR_BACKGROUND);
	int colour_background		= colour_background_t->value->int32;
	persist_write_int(MESSAGE_KEY_COLOUR_BACKGROUND, colour_background);	
	#if defined(PBL_COLOR)
		Tuple *colour_hand_hour_t	= dict_find(iter, MESSAGE_KEY_COLOUR_HAND_HOUR);
		int colour_hand_hour		= colour_hand_hour_t->value->int32;
		persist_write_int(MESSAGE_KEY_COLOUR_HAND_HOUR, colour_hand_hour);
		Tuple *colour_hand_minute_t = dict_find(iter, MESSAGE_KEY_COLOUR_HAND_MINUTE);
		int colour_hand_minute		= colour_hand_minute_t->value->int32;
		persist_write_int(MESSAGE_KEY_COLOUR_HAND_MINUTE, colour_hand_minute);
		Tuple *colour_weekday_t		= dict_find(iter, MESSAGE_KEY_COLOUR_WEEKDAY);
		int colour_weekday			= colour_weekday_t->value->int32;
		persist_write_int(MESSAGE_KEY_COLOUR_WEEKDAY, colour_weekday);
		Tuple *colour_day_t			= dict_find(iter, MESSAGE_KEY_COLOUR_DAY);
		int colour_day				= colour_day_t->value->int32;
		persist_write_int(MESSAGE_KEY_COLOUR_DAY, colour_day);
		Tuple *colour_month_t		= dict_find(iter, MESSAGE_KEY_COLOUR_MONTH);
		int colour_month			= colour_month_t->value->int32;
		persist_write_int(MESSAGE_KEY_COLOUR_MONTH, colour_month);
// Bluetooth
		Tuple *colour_bluetooth_t	= dict_find(iter, MESSAGE_KEY_COLOUR_BLUETOOTH);
		int colour_bluetooth		= colour_bluetooth_t->value->int32;
		persist_write_int(MESSAGE_KEY_COLOUR_BLUETOOTH, colour_bluetooth);
	#endif
// 	Tuple *select_bluetooth_diconnect_t = dict_find(iter, MESSAGE_KEY_SELECT_BLUETOOTH_DISCONNECT);
// 	char *select_bluetooth_disconnect = select_bluetooth_diconnect_t->value->cstring;
// 	persist_write_string(MESSAGE_KEY_SELECT_BLUETOOTH_DISCONNECT, select_bluetooth_disconnect);
		
// Set Colours
	layer_mark_dirty(s_background_layer); 	// Update background
	layer_mark_dirty(s_hands_layer); 		// Update Hands
	text_layer_set_text_color(s_day_label, PBL_IF_BW_ELSE(gcolor_legible_over(GColorFromHEX(colour_background)), GColorFromHEX(colour_day)));
	bluetooth_callback(connection_service_peek_pebble_app_connection());
//	APP_LOG(APP_LOG_LEVEL_DEBUG, select_battery_percent);
}

void unobstructed_change(AnimationProgress progress, void* data) {
	GRect bounds = layer_get_unobstructed_bounds(window_get_root_layer(window));

	GPoint center = grect_center_point(&bounds);
	gpath_move_to(s_minute_arrow, center);
	gpath_move_to(s_hour_arrow, center);
	
	layer_set_frame(text_layer_get_layer(s_weekday_label),GRect(10, bounds.size.h * 6/32, bounds.size.w - 20, 30)); // Top
	layer_set_frame(text_layer_get_layer(s_day_label),GRect(bounds.size.w - 26, bounds.size.h/2 - 15, 25, 30));
	layer_set_frame(text_layer_get_layer(s_month_label),GRect(10, bounds.size.h * 5/8, bounds.size.w - 20, 30));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////// update //////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

static void bg_update_proc(Layer *layer, GContext *ctx) {
	bool message = persist_read_bool(key);	
	
	int colour_background = persist_read_int(MESSAGE_KEY_COLOUR_BACKGROUND);
	if(message) {
		graphics_context_set_fill_color(ctx, GColorFromHEX(colour_background));		// Background
		graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
		graphics_context_set_fill_color(ctx, gcolor_legible_over(GColorFromHEX(colour_background)));		// Markers
	} else {
		graphics_context_set_fill_color(ctx, GColorBlack);		// Background
		graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
		graphics_context_set_fill_color(ctx, GColorWhite);		// Markers
	}
	for (int i = 0; i < NUM_CLOCK_TICKS; ++i) {
		gpath_draw_filled(ctx, s_tick_paths[i]);
	}
}

static void hands_update_proc(Layer *layer, GContext *ctx) {
	GRect bounds = layer_get_unobstructed_bounds(layer);
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	bool message = persist_read_bool(key);

// Hour
	int colour_background = persist_read_int(MESSAGE_KEY_COLOUR_BACKGROUND);
	int colour_hand_hour = persist_read_int(MESSAGE_KEY_COLOUR_HAND_HOUR);
	int colour_hand_minute = persist_read_int(MESSAGE_KEY_COLOUR_HAND_MINUTE);
	
	if(message) {
		graphics_context_set_fill_color(ctx, PBL_IF_BW_ELSE(gcolor_legible_over(GColorFromHEX(colour_background)), GColorFromHEX(colour_hand_hour)));
		graphics_context_set_stroke_color(ctx, GColorFromHEX(colour_background));
	} else {
		graphics_context_set_fill_color(ctx, GColorWhite);
		graphics_context_set_stroke_color(ctx, GColorBlack);
	}
	
	gpath_rotate_to(s_hour_arrow, (TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6)); // (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6)
	gpath_draw_filled(ctx, s_hour_arrow);
	gpath_draw_outline(ctx, s_hour_arrow);

// Minute
	if(message) {
		graphics_context_set_fill_color(ctx, PBL_IF_BW_ELSE(gcolor_legible_over(GColorFromHEX(colour_background)), GColorFromHEX(colour_hand_minute)));
		graphics_context_set_stroke_color(ctx, GColorFromHEX(colour_background));
	} else {
		graphics_context_set_fill_color(ctx, GColorWhite);
		graphics_context_set_stroke_color(ctx, GColorBlack);
	}

	gpath_rotate_to(s_minute_arrow, TRIG_MAX_ANGLE * t->tm_min / 60); // t->tm_min / 60
	gpath_draw_filled(ctx, s_minute_arrow);
	gpath_draw_outline(ctx, s_minute_arrow);

// Dot
	if(message) {
		graphics_context_set_fill_color(ctx, PBL_IF_BW_ELSE(gcolor_legible_over(GColorFromHEX(colour_background)), GColorFromHEX(colour_hand_minute)));
	} else {
		graphics_context_set_fill_color(ctx, GColorWhite);
	}
	graphics_fill_rect(ctx, GRect(bounds.size.w / 2 - 1, bounds.size.h / 2 - 1, 3, 3), 0, GCornerNone);
}

static void date_update_proc(Layer *layer, GContext *ctx) {
	time_t now = time(NULL);
	struct tm *time = localtime(&now);

// Weekday
	strftime(s_weekday_buffer, sizeof(s_weekday_buffer), "%A", time);
	text_layer_set_text(s_weekday_label, s_weekday_buffer);

// Day
	strftime(s_date_buffer, sizeof(s_date_buffer), "%d", time);
	text_layer_set_text(s_day_label, s_date_buffer);

// Month
	strftime(s_month_buffer, sizeof(s_month_buffer), "%B", time);
	text_layer_set_text(s_month_label, s_month_buffer);
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
	layer_mark_dirty(window_get_root_layer(window));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////// Load & Unload ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_unobstructed_bounds(window_layer);

	s_background_layer = layer_create(bounds);
	layer_set_update_proc(s_background_layer, bg_update_proc);
	layer_add_child(window_layer, s_background_layer);

	s_date_layer = layer_create(bounds);
	layer_set_update_proc(s_date_layer, date_update_proc);
	layer_add_child(window_layer, s_date_layer);
	
// Locations
	s_weekday_label = text_layer_create(GRect(10, bounds.size.h * 6/32, bounds.size.w - 20, 30)); // Top
	s_day_label = text_layer_create(GRect(bounds.size.w - 26, bounds.size.h/2 - 15, 25, 30));
	s_month_label = text_layer_create(GRect(10, bounds.size.h * 5/8, bounds.size.w - 20, 30));

// Weekday
	text_layer_set_text_alignment(s_weekday_label, GTextAlignmentCenter);
	text_layer_set_background_color(s_weekday_label, GColorClear);
	text_layer_set_font(s_weekday_label, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_BEBAS_NEUE_REGULAR_24)));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weekday_label));
	text_layer_set_text(s_weekday_label, s_weekday_buffer);

// Date
	text_layer_set_text_alignment(s_day_label, GTextAlignmentRight);
	text_layer_set_background_color(s_day_label, GColorClear);
	text_layer_set_font(s_day_label, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_BEBAS_NEUE_REGULAR_24)));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_day_label));
	text_layer_set_text(s_day_label, s_date_buffer);
	
// Month
	text_layer_set_text_alignment(s_month_label, GTextAlignmentCenter);
	text_layer_set_background_color(s_month_label, GColorClear);
	text_layer_set_font(s_month_label, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_BEBAS_NEUE_REGULAR_24)));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_month_label));
	text_layer_set_text(s_month_label, s_month_buffer);
	
// Set Colours
	int colour_background = persist_read_int(MESSAGE_KEY_COLOUR_BACKGROUND);
	int colour_day = persist_read_int(MESSAGE_KEY_COLOUR_DAY);
	if(colour_background || colour_day) {
		text_layer_set_text_color(s_day_label, GColorFromHEX(colour_day));
	} else {
		text_layer_set_text_color(s_day_label, GColorWhite);
	}
	
// Hands
	s_hands_layer = layer_create(bounds);
	layer_set_update_proc(s_hands_layer, hands_update_proc);
	layer_add_child(window_layer, s_hands_layer);
	
	bluetooth_callback(connection_service_peek_pebble_app_connection());
}

static void window_unload(Window *window) {
	layer_destroy(s_background_layer);
	layer_destroy(s_date_layer);

	text_layer_destroy(s_weekday_label);
	text_layer_destroy(s_day_label);
	text_layer_destroy(s_month_label);

	layer_destroy(s_hands_layer);
}

static void init() {
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	window_stack_push(window, true);
	
	UnobstructedAreaHandlers handlers = {
		.change = unobstructed_change,
	};
	unobstructed_area_service_subscribe(handlers, NULL);
	
	s_weekday_buffer[0] = '\0';
	s_date_buffer[0] 	= '\0';
	s_month_buffer[0] 	= '\0';

// init hand paths
	s_minute_arrow = gpath_create(&MINUTE_HAND_POINTS);
	s_hour_arrow = gpath_create(&HOUR_HAND_POINTS);

	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_unobstructed_bounds(window_layer);
	GPoint center = grect_center_point(&bounds);
	gpath_move_to(s_minute_arrow, center);
	gpath_move_to(s_hour_arrow, center);
	
	for (int i = 0; i < NUM_CLOCK_TICKS; ++i) {
		s_tick_paths[i] = gpath_create(&ANALOG_BG_POINTS[i]);
	}
	
	connection_service_subscribe((ConnectionHandlers) {
  		.pebble_app_connection_handler = bluetooth_callback
	});
	
	const int inbox_size = 128;
	const int outbox_size = 128;
	app_message_register_inbox_received(inbox_received_handler);
	app_message_open(inbox_size, outbox_size);
	
	tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
}

static void deinit() {
	gpath_destroy(s_minute_arrow);
	gpath_destroy(s_hour_arrow);

	for (int i = 0; i < NUM_CLOCK_TICKS; ++i) {
		gpath_destroy(s_tick_paths[i]);
	}

	tick_timer_service_unsubscribe();
	window_destroy(window);
}

int main() {
	init();
	app_event_loop();
	deinit();
}
