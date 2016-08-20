#include <pebble.h>
#include "main.h"

static Window *window;
static Layer *s_simple_bg_layer, *s_date_layer, *s_hands_layer;
static TextLayer *s_weekday_label, *s_day_label;

static GPath *s_tick_paths[NUM_CLOCK_TICKS];
static GPath *s_minute_arrow, *s_hour_arrow;
static char s_weekday_buffer[16], s_day_buffer[16];

static void bg_update_proc(Layer *layer, GContext *ctx) {
	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
	graphics_context_set_fill_color(ctx, GColorWhite);
	for (int i = 0; i < NUM_CLOCK_TICKS; ++i) {
		gpath_draw_filled(ctx, s_tick_paths[i]);
	}
}

static void hands_update_proc(Layer *layer, GContext *ctx) {
	GRect bounds = layer_get_bounds(layer);

	time_t now = time(NULL);
	struct tm *t = localtime(&now);

// Hour/Minute hand
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_context_set_stroke_color(ctx, gcolor_legible_over(GColorWhite));

	gpath_rotate_to(s_minute_arrow, TRIG_MAX_ANGLE * t->tm_min / 60);
	gpath_draw_filled(ctx, s_minute_arrow);
	gpath_draw_outline(ctx, s_minute_arrow);

	gpath_rotate_to(s_hour_arrow, (TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6));
	gpath_draw_filled(ctx, s_hour_arrow);
	gpath_draw_outline(ctx, s_hour_arrow);

// Dot
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_rect(ctx, GRect(bounds.size.w / 2 - 1, bounds.size.h / 2 - 1, 3, 3), 0, GCornerNone);
}

static void date_update_proc(Layer *layer, GContext *ctx) {
	time_t now = time(NULL);
	struct tm *t = localtime(&now);

	strftime(s_weekday_buffer, sizeof(s_weekday_buffer), "%A", t);
	text_layer_set_text(s_weekday_label, s_weekday_buffer);
	
	strftime(s_day_buffer, sizeof(s_day_buffer), "%d", t);
	text_layer_set_text(s_day_label, s_day_buffer);
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
	layer_mark_dirty(window_get_root_layer(window));
}

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	s_simple_bg_layer = layer_create(bounds);
	layer_set_update_proc(s_simple_bg_layer, bg_update_proc);
	layer_add_child(window_layer, s_simple_bg_layer);

	s_date_layer = layer_create(bounds);
	layer_set_update_proc(s_date_layer, date_update_proc);
	layer_add_child(window_layer, s_date_layer);
	
// Weekday
	s_weekday_label = text_layer_create(GRect(10, bounds.size.h * 5/8, bounds.size.w - 20, 30));
	text_layer_set_text(s_weekday_label, s_weekday_buffer);
	text_layer_set_text_alignment(s_weekday_label, GTextAlignmentCenter);
	text_layer_set_background_color(s_weekday_label, GColorClear);
	text_layer_set_text_color(s_weekday_label, GColorWhite);
	text_layer_set_font(s_weekday_label, fonts_get_system_font(FONT_KEY_GOTHIC_24));

	layer_add_child(s_date_layer, text_layer_get_layer(s_weekday_label));

// Day
	s_day_label = text_layer_create(GRect(bounds.size.w - 26, bounds.size.h/2 - 15, 25, 30));
	text_layer_set_text(s_day_label, s_day_buffer);
	text_layer_set_text_alignment(s_day_label, GTextAlignmentRight);
	text_layer_set_background_color(s_day_label, GColorClear);
	text_layer_set_text_color(s_day_label, GColorWhite);
	text_layer_set_font(s_day_label, fonts_get_system_font(FONT_KEY_GOTHIC_24));

	layer_add_child(s_date_layer, text_layer_get_layer(s_day_label));
	
// Hands
	s_hands_layer = layer_create(bounds);
	layer_set_update_proc(s_hands_layer, hands_update_proc);
	layer_add_child(window_layer, s_hands_layer);
}

static void window_unload(Window *window) {
	layer_destroy(s_simple_bg_layer);
	layer_destroy(s_date_layer);

	text_layer_destroy(s_weekday_label);
	text_layer_destroy(s_day_label);

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
	s_day_buffer[0] 	= '\0';

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
