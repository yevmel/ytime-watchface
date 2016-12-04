#include <pebble.h>

static Window *main_window;
static TextLayer *time_layer;
static TextLayer *date_layer;
static TextLayer *day_layer;

static struct tm *local_time = NULL;
static void update_local_time() {
    time_t t = time(NULL);
    local_time = localtime(&t);
}

static TextLayer* create_text_layer(
        Layer *parent_layer, 
        const char *font_key, 
        int16_t origin_x, 
        int16_t origin_y, 
        int16_t width, 
        int16_t height) {
    TextLayer* text_layer = text_layer_create(GRect(origin_x, origin_y, width, height));
    GFont font = fonts_get_system_font(font_key);

    text_layer_set_background_color(text_layer, GColorRed);
    text_layer_set_text_color(text_layer, GColorWhite);
    text_layer_set_font(text_layer, font);
    text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
    
    Layer *layer = text_layer_get_layer(text_layer);
    layer_add_child(parent_layer, layer);

    return text_layer;
}

static void update_date() {
    static char text_layer_buffer[32];
    strftime(text_layer_buffer, 32, "%d %B %Y", local_time);
    text_layer_set_text(date_layer, text_layer_buffer);
}

static void update_day() {
    static char text_layer_buffer[12];
    strftime(text_layer_buffer, 12, "%A", local_time);
    text_layer_set_text(day_layer, text_layer_buffer);
}

static void update_time() {
    static char text_layer_buffer[6];
    strftime(text_layer_buffer, 6, clock_is_24h_style() ? "%H:%M" : "%I:%M", local_time);
    text_layer_set_text(time_layer, text_layer_buffer);
}

static void tick_handler(struct tm *local_time, TimeUnits units_changed) {
    if (units_changed & MINUTE_UNIT) {
        update_local_time();
        update_time();
    }

/* not sure if useful
    if (units_changed & HOUR_UNIT) {
        if (!quiet_time_is_active()) {
            vibes_short_pulse();
        }
    }
*/

    if (units_changed & DAY_UNIT) {
        update_day();
        update_date();
    }
}

static void main_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    time_layer = create_text_layer(window_layer, FONT_KEY_BITHAM_42_BOLD, 0, 32, bounds.size.w, 50);
    date_layer = create_text_layer(window_layer, FONT_KEY_GOTHIC_18, 0, 82, bounds.size.w, 26);
    day_layer = create_text_layer(window_layer, FONT_KEY_GOTHIC_18, 0, 112, bounds.size.w, 26);
    
    update_local_time();
    update_time();
    update_date();
    update_day();
}

static void main_window_unload(Window *window) {
    text_layer_destroy(time_layer);
    text_layer_destroy(date_layer);
    text_layer_destroy(day_layer);
}

static void init() {
    main_window = window_create();
    window_set_background_color(main_window, GColorRed);
    window_set_window_handlers(main_window, (WindowHandlers) {
        .load   = main_window_load,
        .unload = main_window_unload
    });

    window_stack_push(main_window, true);

    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void cleanup() {
    window_destroy(main_window);
}

int main(void) {
    setlocale(LC_TIME, "");

    init();
    app_event_loop();
    cleanup();
}
