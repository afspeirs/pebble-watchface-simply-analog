#include <pebble.h>
#include "main.h"

#define SETTINGS_KEY 1				// Persistent storage key

static Window *window;
static Layer *s_background_layer, *s_date_layer, *s_hands_layer;
static TextLayer *s_weekday_label, *s_date_label, *s_month_label;
static char s_weekday_buffer[16], s_date_buffer[16], s_month_buffer[16];
static GPath *s_minute_arrow, *s_hour_arrow, *s_tick_paths[NUM_CLOCK_TICKS];
static BitmapLayer *s_layer_battery, *s_layer_bluetooth, *s_layer_quiet;
static GBitmap *s_bitmap_battery, *s_bitmap_bluetooth, *s_bitmap_quiet;
static bool appStarted = false;

typedef struct ClaySettings {
	GColor ColourBackground;
	GColor ColourHour;
	GColor ColourMinute;
	GColor ColourWeekday;
	GColor ColourDate;
	GColor ColourMonth;
	bool ToggleBluetooth;
	bool ToggleBluetoothQuietTime;
	int SelectBluetooth;
	int SelectBatteryPercent;
	bool TogglePowerSave;
	bool ToggleSuffix;
} ClaySettings;						// Define our settings struct

static ClaySettings settings;		// An instance of the struct

static void config_default() {
	settings.ColourBackground   = GColorBlack;
	settings.ColourHour         = PBL_IF_BW_ELSE(GColorWhite,GColorChromeYellow);
	settings.ColourMinute       = GColorWhite;
	settings.ColourWeekday      = GColorWhite;
	settings.ColourDate         = GColorWhite;
	settings.ColourMonth        = GColorWhite;
	settings.ToggleBluetooth    = true;
	settings.ToggleBluetoothQuietTime = false;
	settings.SelectBluetooth    = 2;
	settings.SelectBatteryPercent = 0;
	settings.TogglePowerSave    = false;
	settings.ToggleSuffix       = false;
}

static void config_load() {
	config_default();													// Load the default settings
	persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));		// Read settings from persistent storage, if they exist
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
////  Methods  ///////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void getIcon(GBitmap *bitmap, BitmapLayer *bitmapLayer, int imageBlack, int imageWhite) {
	gbitmap_destroy(bitmap);
	if (gcolor_equal(gcolor_legible_over(settings.ColourBackground), GColorBlack)) {
		bitmap = gbitmap_create_with_resource(imageBlack);
	} else {
		bitmap = gbitmap_create_with_resource(imageWhite);
	}
	bitmap_layer_set_bitmap(bitmapLayer, bitmap);	
}

// Sets the colours of the background, hands and text
void setColours() {
	layer_mark_dirty(s_background_layer); 	// Update background
	layer_mark_dirty(s_hands_layer); 		// Update Hands
	text_layer_set_text_color(s_weekday_label, PBL_IF_BW_ELSE(gcolor_legible_over(settings.ColourBackground), settings.ColourWeekday));  // Set Weekday
	text_layer_set_text_color(s_date_label, PBL_IF_BW_ELSE(gcolor_legible_over(settings.ColourBackground), settings.ColourDate));        // Set Date
	text_layer_set_text_color(s_month_label, PBL_IF_BW_ELSE(gcolor_legible_over(settings.ColourBackground), settings.ColourMonth));      // Set Month
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////// Callbacks ///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

static void battery_callback(BatteryChargeState state) {
	if(state.charge_percent <= settings.SelectBatteryPercent) {
		getIcon(s_bitmap_battery, s_layer_battery, RESOURCE_ID_IMAGE_BATTERY_BLACK, RESOURCE_ID_IMAGE_BATTERY_WHITE);
		layer_set_hidden(bitmap_layer_get_layer(s_layer_battery), false);	// Visible
	} else {
		layer_set_hidden(bitmap_layer_get_layer(s_layer_battery), true);	// Hidden
	}
	
	// Move battery when in quiet time, and move to original location if not
	if(quiet_time_is_active()) {
		#if PBL_DISPLAY_HEIGHT == 180			// Chalk
			layer_set_frame(bitmap_layer_get_layer(s_layer_battery),GRect(84, 17, 13,  6));	// battery
		#else									// Aplite, Basalt, Diorite
			layer_set_frame(bitmap_layer_get_layer(s_layer_battery),GRect(22, 4, 13,  6));	// battery
		#endif
	} else {
		#if PBL_DISPLAY_HEIGHT == 180			// Chalk
			layer_set_frame(bitmap_layer_get_layer(s_layer_battery),GRect(84, 10, 13,  6));	// battery
		#else									// Aplite, Basalt, Diorite
			layer_set_frame(bitmap_layer_get_layer(s_layer_battery),GRect(6, 4, 13,  6));	// battery
		#endif
	}
}

static void bluetooth_callback(bool connected) {													  	
	if(!connected) {
		if(appStarted && !(quiet_time_is_active() && !settings.ToggleBluetoothQuietTime)) {
			if(settings.SelectBluetooth == 0) { }								// No vibration 
			else if(settings.SelectBluetooth == 1) { vibes_short_pulse(); }		// Short vibration
			else if(settings.SelectBluetooth == 2) { vibes_long_pulse(); }		// Long vibration
			else if(settings.SelectBluetooth == 3) { vibes_double_pulse(); }	// Double vibration
			else { vibes_long_pulse(); }					 // Default			// Long Vibration
		}
		if(settings.ToggleBluetooth) {
			getIcon(s_bitmap_bluetooth, s_layer_bluetooth, RESOURCE_ID_IMAGE_BLUETOOTH_BLACK, RESOURCE_ID_IMAGE_BLUETOOTH_WHITE);
			layer_set_hidden(bitmap_layer_get_layer(s_layer_bluetooth), false);	// Visible	
		}
	} else {
		layer_set_hidden(bitmap_layer_get_layer(s_layer_bluetooth), true);	// Hidden
	}	
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
// Colours
	Tuple *bg_colour_t = dict_find(iter, MESSAGE_KEY_COLOUR_BACKGROUND);
	if(bg_colour_t) { settings.ColourBackground = GColorFromHEX(bg_colour_t->value->int32); }
	Tuple *hr_colour_t = dict_find(iter, MESSAGE_KEY_COLOUR_HOUR);
	if(hr_colour_t) { settings.ColourHour = GColorFromHEX(hr_colour_t->value->int32); }
	Tuple *mn_colour_t = dict_find(iter, MESSAGE_KEY_COLOUR_MINUTE);
	if(mn_colour_t) { settings.ColourMinute = GColorFromHEX(mn_colour_t->value->int32); }
	Tuple *wd_colour_t = dict_find(iter, MESSAGE_KEY_COLOUR_WEEKDAY);
	if(wd_colour_t) { settings.ColourWeekday = GColorFromHEX(wd_colour_t->value->int32); }
	Tuple *dt_colour_t = dict_find(iter, MESSAGE_KEY_COLOUR_DATE);
	if(dt_colour_t) { settings.ColourDate = GColorFromHEX(dt_colour_t->value->int32); }
	Tuple *month_colour_t = dict_find(iter, MESSAGE_KEY_COLOUR_MONTH);
	if(month_colour_t) { settings.ColourMonth = GColorFromHEX(month_colour_t->value->int32); }
// Bluetooth
	Tuple *bt_toggle_t = dict_find(iter, MESSAGE_KEY_TOGGLE_BLUETOOTH);
	if(bt_toggle_t) { settings.ToggleBluetooth = bt_toggle_t->value->int32 == 1; }
	Tuple *bq_toggle_t = dict_find(iter, MESSAGE_KEY_TOGGLE_BLUETOOTH_QUIET_TIME);
	if(bq_toggle_t) { settings.ToggleBluetoothQuietTime = bq_toggle_t->value->int32 == 1; }
	Tuple *bt_select_t = dict_find(iter, MESSAGE_KEY_SELECT_BLUETOOTH);
	if(bt_select_t) { settings.SelectBluetooth = atoi(bt_select_t->value->cstring); }
// Battery
	Tuple *bp_select_t = dict_find(iter, MESSAGE_KEY_SELECT_BATTERY_PERCENT);
	if(bp_select_t) { settings.SelectBatteryPercent = atoi(bp_select_t->value->cstring); }
	
  	persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));		// Write settings to persistent storage
	
// Update watchface with new settings
	battery_callback(battery_state_service_peek());
	appStarted = false;
	bluetooth_callback(connection_service_peek_pebble_app_connection());
	appStarted = true;
	setColours();

	if(quiet_time_is_active()) {
		getIcon(s_bitmap_quiet, s_layer_quiet, RESOURCE_ID_IMAGE_QUIET_TIME_BLACK, RESOURCE_ID_IMAGE_QUIET_TIME_WHITE);
		layer_set_hidden(bitmap_layer_get_layer(s_layer_quiet), false);	// Visible
	} else {
		layer_set_hidden(bitmap_layer_get_layer(s_layer_quiet), true);	// Hidden
	}
}

void unobstructed_change(AnimationProgress progress, void* data) {
	GRect bounds = layer_get_unobstructed_bounds(window_get_root_layer(window));

	GPoint center = grect_center_point(&bounds);
	gpath_move_to(s_minute_arrow, center);
	gpath_move_to(s_hour_arrow, center);
	
	layer_set_frame(text_layer_get_layer(s_weekday_label),GRect(10, bounds.size.h * 6/32, bounds.size.w - 20, 30)); // Top
	layer_set_frame(text_layer_get_layer(s_date_label),GRect(bounds.size.w - 26, bounds.size.h/2 - 15, 25, 30));
	layer_set_frame(text_layer_get_layer(s_month_label),GRect(10, bounds.size.h * 5/8, bounds.size.w - 20, 30));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////// update //////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

static void bg_update_proc(Layer *layer, GContext *ctx) {	
	graphics_context_set_fill_color(ctx, settings.ColourBackground);                          // Background
	graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
	graphics_context_set_fill_color(ctx, gcolor_legible_over(settings.ColourBackground));     // Markers

	for (int i = 0; i < NUM_CLOCK_TICKS; ++i) {
		gpath_draw_filled(ctx, s_tick_paths[i]);
	}
}

static void hands_update_proc(Layer *layer, GContext *ctx) {
	GRect bounds = layer_get_unobstructed_bounds(layer);
	time_t now = time(NULL);
	struct tm *t = localtime(&now);

// Hour
	graphics_context_set_fill_color(ctx, PBL_IF_BW_ELSE(gcolor_legible_over(settings.ColourBackground), settings.ColourHour));
	graphics_context_set_stroke_color(ctx, settings.ColourBackground);

	gpath_rotate_to(s_hour_arrow, (TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6)); // (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6)
	gpath_draw_filled(ctx, s_hour_arrow);
	gpath_draw_outline(ctx, s_hour_arrow);

// Minute
	graphics_context_set_fill_color(ctx, PBL_IF_BW_ELSE(gcolor_legible_over(settings.ColourBackground), settings.ColourMinute));
	graphics_context_set_stroke_color(ctx, settings.ColourBackground);

	gpath_rotate_to(s_minute_arrow, TRIG_MAX_ANGLE * t->tm_min / 60); // t->tm_min / 60
	gpath_draw_filled(ctx, s_minute_arrow);
	gpath_draw_outline(ctx, s_minute_arrow);

// Dot
	graphics_context_set_fill_color(ctx, PBL_IF_BW_ELSE(gcolor_legible_over(settings.ColourBackground), settings.ColourMinute));

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
	text_layer_set_text(s_date_label, s_date_buffer);

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
	s_weekday_label = text_layer_create(GRect(10, bounds.size.h * 6/32, bounds.size.w - 20, 30)); // Weekday
	s_date_label = text_layer_create(GRect(bounds.size.w - 26, bounds.size.h/2 - 15, 25, 30));    // Date
	s_month_label = text_layer_create(GRect(10, bounds.size.h * 5/8, bounds.size.w - 20, 30));    // Month
	s_layer_bluetooth = bitmap_layer_create(GRect(bounds.size.w-13, 3, 7,  11));                  // Bluetooth

// Show Quiet Time icon when active, and move battery 
	if(quiet_time_is_active()) {
		s_layer_battery		= bitmap_layer_create(GRect(22, 4, 13,  6));	// battery
		s_layer_quiet		= bitmap_layer_create(GRect( 6, 2, 10, 10));	// quiet

		getIcon(s_bitmap_quiet, s_layer_quiet, RESOURCE_ID_IMAGE_QUIET_TIME_BLACK, RESOURCE_ID_IMAGE_QUIET_TIME_WHITE);
		layer_set_hidden(bitmap_layer_get_layer(s_layer_quiet), false);	// Visible
	} else {
		s_layer_battery	= bitmap_layer_create(GRect(6, 4, 13,  6));		// battery
		s_layer_quiet	= bitmap_layer_create(GRect(6, 2, 10, 10));		// quiet

		layer_set_hidden(bitmap_layer_get_layer(s_layer_quiet), true);	// Hidden
	}

// Battery Icon
	layer_mark_dirty(bitmap_layer_get_layer(s_layer_battery));
	#if defined(PBL_COLOR)
		bitmap_layer_set_compositing_mode(s_layer_battery, GCompOpSet);	
	#endif
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_layer_battery));

// Bluetooth Icon
	layer_mark_dirty(bitmap_layer_get_layer(s_layer_bluetooth));
	#if defined(PBL_COLOR)
		bitmap_layer_set_compositing_mode(s_layer_bluetooth, GCompOpSet);	
	#endif
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_layer_bluetooth));
	
// Quiet Time Icon
	layer_mark_dirty(bitmap_layer_get_layer(s_layer_quiet));
	#if defined(PBL_COLOR)
		bitmap_layer_set_compositing_mode(s_layer_quiet, GCompOpSet);	
	#endif
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_layer_quiet));

// Weekday
	text_layer_set_text_alignment(s_weekday_label, GTextAlignmentCenter);
	text_layer_set_background_color(s_weekday_label, GColorClear);
	text_layer_set_font(s_weekday_label, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_BEBAS_NEUE_REGULAR_24)));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weekday_label));
	text_layer_set_text(s_weekday_label, s_weekday_buffer);

// Date
	text_layer_set_text_alignment(s_date_label, GTextAlignmentRight);
	text_layer_set_background_color(s_date_label, GColorClear);
	text_layer_set_font(s_date_label, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_BEBAS_NEUE_REGULAR_24)));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_label));
	text_layer_set_text(s_date_label, s_date_buffer);
	
// Month
	text_layer_set_text_alignment(s_month_label, GTextAlignmentCenter);
	text_layer_set_background_color(s_month_label, GColorClear);
	text_layer_set_font(s_month_label, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_BEBAS_NEUE_REGULAR_24)));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_month_label));
	text_layer_set_text(s_month_label, s_month_buffer);
	
// Hands
	s_hands_layer = layer_create(bounds);
	layer_set_update_proc(s_hands_layer, hands_update_proc);
	layer_add_child(window_layer, s_hands_layer);

	battery_callback(battery_state_service_peek());
	appStarted = false;
	bluetooth_callback(connection_service_peek_pebble_app_connection());		// Sets date colours (and detects if a phone is connected)
	appStarted = true;
	setColours();
}

static void window_unload(Window *window) {
	layer_destroy(s_background_layer);
	layer_destroy(s_date_layer);

	text_layer_destroy(s_weekday_label);
	text_layer_destroy(s_date_label);
	text_layer_destroy(s_month_label);

	layer_destroy(s_hands_layer);
}

static void init() {
	config_load();

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
	
	app_message_register_inbox_received(inbox_received_handler);
	app_message_open(256, 256);
	
	battery_state_service_subscribe(battery_callback);
	battery_callback(battery_state_service_peek());
	
	tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
}

static void deinit() {
	gpath_destroy(s_minute_arrow);
	gpath_destroy(s_hour_arrow);
	gbitmap_destroy(s_bitmap_bluetooth);
	bitmap_layer_destroy(s_layer_bluetooth);

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