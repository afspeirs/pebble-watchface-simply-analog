#include <pebble.h>
#include "main.h"

static Window *window;
static Layer *s_background_layer, *s_date_layer, *s_hands_layer;
static TextLayer *s_weekday_label, *s_day_label, *s_month_label;
static char s_weekday_buffer[16], s_date_buffer[16], s_month_buffer[16];
static GPath *s_tick_paths[NUM_CLOCK_TICKS];
static GPath *s_minute_arrow, *s_hour_arrow;

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////// Callbacks ///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
// Colours
	Tuple *colour_background_t	= dict_find(iter, MESSAGE_KEY_COLOUR_BACKGROUND);
 	Tuple *colour_hand_hour_t	= dict_find(iter, MESSAGE_KEY_COLOUR_HAND_HOUR);
	Tuple *colour_hand_minute_t = dict_find(iter, MESSAGE_KEY_COLOUR_HAND_MINUTE);
	Tuple *colour_weekday_t		= dict_find(iter, MESSAGE_KEY_COLOUR_WEEKDAY);
	Tuple *colour_day_t			= dict_find(iter, MESSAGE_KEY_COLOUR_DAY);
	Tuple *colour_month_t		= dict_find(iter, MESSAGE_KEY_COLOUR_MONTH);
// 	Tuple *colour_bluetooth_t	= dict_find(iter, MESSAGE_KEY_COLOUR_BLUETOOTH);
    int colour_background		= colour_background_t->value->int32;
 	int colour_hand_hour		= colour_hand_hour_t->value->int32;
	int colour_hand_minute		= colour_hand_minute_t->value->int32;
	int colour_weekday			= colour_weekday_t->value->int32;
	int colour_day				= colour_day_t->value->int32;
	int colour_month			= colour_month_t->value->int32;
// 	int colour_bluetooth = colour_bluetooth_t->value->int32;
	persist_write_int(MESSAGE_KEY_COLOUR_BACKGROUND, colour_background);
	persist_write_int(MESSAGE_KEY_COLOUR_HAND_HOUR, colour_hand_hour);
	persist_write_int(MESSAGE_KEY_COLOUR_HAND_MINUTE, colour_hand_minute);
	persist_write_int(MESSAGE_KEY_COLOUR_WEEKDAY, colour_weekday);
	persist_write_int(MESSAGE_KEY_COLOUR_DAY, colour_day);
	persist_write_int(MESSAGE_KEY_COLOUR_MONTH, colour_month);
// 	persist_write_int(MESSAGE_KEY_COLOUR_BLUETOOTH, colour_bluetooth);	
	
// Set Colours
	layer_mark_dirty(s_background_layer); //update background
	layer_mark_dirty(s_hands_layer); //update Hands
	text_layer_set_text_color(s_weekday_label, GColorFromHEX(colour_weekday));	// Weekday
	text_layer_set_text_color(s_day_label, GColorFromHEX(colour_day));			// Date
	text_layer_set_text_color(s_month_label, GColorFromHEX(colour_month));		// Month
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////// Main ////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

static void bg_update_proc(Layer *layer, GContext *ctx) {
	int colour_background = persist_read_int(MESSAGE_KEY_COLOUR_BACKGROUND);
	if(colour_background) {
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
	GRect bounds = layer_get_bounds(layer);

	time_t now = time(NULL);
	struct tm *t = localtime(&now);

// Hour
	int colour_background = persist_read_int(MESSAGE_KEY_COLOUR_BACKGROUND);
	int colour_hand_hour = persist_read_int(MESSAGE_KEY_COLOUR_HAND_HOUR);
	int colour_hand_minute = persist_read_int(MESSAGE_KEY_COLOUR_HAND_MINUTE);
	
	if(colour_background || colour_hand_hour) {
		graphics_context_set_fill_color(ctx, GColorFromHEX(colour_hand_hour));
		graphics_context_set_stroke_color(ctx, GColorFromHEX(colour_background));
	} else {
		graphics_context_set_fill_color(ctx, GColorWhite);
		graphics_context_set_stroke_color(ctx, GColorBlack);
	}
	
	gpath_rotate_to(s_hour_arrow, (TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6)); // (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6)
	gpath_draw_filled(ctx, s_hour_arrow);
	gpath_draw_outline(ctx, s_hour_arrow);

// Minute
	if(colour_background || colour_hand_minute) {
		graphics_context_set_fill_color(ctx, GColorFromHEX(colour_hand_minute));
		graphics_context_set_stroke_color(ctx, GColorFromHEX(colour_background));
	} else {
		graphics_context_set_fill_color(ctx, GColorWhite);
		graphics_context_set_stroke_color(ctx, GColorBlack);
	}

	gpath_rotate_to(s_minute_arrow, TRIG_MAX_ANGLE * t->tm_min / 60); // t->tm_min / 60
	gpath_draw_filled(ctx, s_minute_arrow);
	gpath_draw_outline(ctx, s_minute_arrow);

// Dot
	if(colour_background || colour_hand_hour) {
		graphics_context_set_fill_color(ctx, GColorFromHEX(colour_hand_hour));
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

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	s_background_layer = layer_create(bounds);
	layer_set_update_proc(s_background_layer, bg_update_proc);
	layer_add_child(window_layer, s_background_layer);

	s_date_layer = layer_create(bounds);
	layer_set_update_proc(s_date_layer, date_update_proc);
	layer_add_child(window_layer, s_date_layer);
	
// Weekday
	s_weekday_label = text_layer_create(GRect(10, bounds.size.h * 6/32, bounds.size.w - 20, 30)); // Top
// 	s_weekday_label = text_layer_create(GRect(10, bounds.size.h * 5/8, bounds.size.w - 20, 30)); // Bottom
	text_layer_set_text_alignment(s_weekday_label, GTextAlignmentCenter);
	text_layer_set_background_color(s_weekday_label, GColorClear);
	text_layer_set_font(s_weekday_label, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_BEBAS_NEUE_REGULAR_24)));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weekday_label));
	text_layer_set_text(s_weekday_label, s_weekday_buffer);

// Date
	s_day_label = text_layer_create(GRect(bounds.size.w - 26, bounds.size.h/2 - 15, 25, 30));
	text_layer_set_text_alignment(s_day_label, GTextAlignmentRight);
	text_layer_set_background_color(s_day_label, GColorClear);
	text_layer_set_font(s_day_label, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_BEBAS_NEUE_REGULAR_24)));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_day_label));
	text_layer_set_text(s_day_label, s_date_buffer);
	
// Month
	s_month_label = text_layer_create(GRect(10, bounds.size.h * 5/8, bounds.size.w - 20, 30)); // Bottom
	text_layer_set_text_alignment(s_month_label, GTextAlignmentCenter);
// 	s_month_label = text_layer_create(GRect( 0, bounds.size.h / 2 - 15, 50, 30)); // Left
// 	text_layer_set_text_alignment(s_month_label, GTextAlignmentLeft);
	text_layer_set_background_color(s_month_label, GColorClear);
	text_layer_set_font(s_month_label, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_BEBAS_NEUE_REGULAR_24)));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_month_label));
	text_layer_set_text(s_month_label, s_month_buffer);
	
// Set Colours
	int colour_background = persist_read_int(MESSAGE_KEY_COLOUR_BACKGROUND);
	int colour_weekday = persist_read_int(MESSAGE_KEY_COLOUR_WEEKDAY);
	int colour_day = persist_read_int(MESSAGE_KEY_COLOUR_DAY);
	int colour_month = persist_read_int(MESSAGE_KEY_COLOUR_MONTH);
	if(colour_background || colour_weekday || colour_day || colour_month) {
		text_layer_set_text_color(s_weekday_label, GColorFromHEX(colour_weekday));
		text_layer_set_text_color(s_day_label, GColorFromHEX(colour_day));
		text_layer_set_text_color(s_month_label, GColorFromHEX(colour_month));
	} else {
		text_layer_set_text_color(s_weekday_label, GColorWhite);
		text_layer_set_text_color(s_day_label, GColorWhite);
		text_layer_set_text_color(s_month_label, GColorWhite);
	}
	
// Hands
	s_hands_layer = layer_create(bounds);
	layer_set_update_proc(s_hands_layer, hands_update_proc);
	layer_add_child(window_layer, s_hands_layer);
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
	
	s_weekday_buffer[0] = '\0';
	s_date_buffer[0] 	= '\0';
	s_month_buffer[0] 	= '\0';

// init hand paths
	s_minute_arrow = gpath_create(&MINUTE_HAND_POINTS);
	s_hour_arrow = gpath_create(&HOUR_HAND_POINTS);

	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);
	GPoint center = grect_center_point(&bounds);
	gpath_move_to(s_minute_arrow, center);
	gpath_move_to(s_hour_arrow, center);
	
	for (int i = 0; i < NUM_CLOCK_TICKS; ++i) {
		s_tick_paths[i] = gpath_create(&ANALOG_BG_POINTS[i]);
	}
	
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
