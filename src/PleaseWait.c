/*
 * PleaseWait.c
 *
 *  Created on: 6 Jul 2013
 *      Author: andrew
 */

#include "pebble_app.h"
#include "pebble_app_info.h"
#include "pebble_fonts.h"
#include "PleaseWait.h"
#include "stdbool.h"
#include "logger.h"
#include "defines.h"
#include "ScreenCapture.h"

static Window s_window;
static TextLayer s_title;
static TextLayer s_body;
static TextLayer s_debug;
static TextLayer s_version;


static void mClickHandler(ClickRecognizerRef recognizer, void *context)
{
	pbl_capture_send();
}

static void mClickConfigProvider(ClickConfig **array_of_ptrs_to_click_configs_to_setup, void *context)
{
#ifdef SCREEN_CAPTURE_ENABLED
	array_of_ptrs_to_click_configs_to_setup[BUTTON_ID_SELECT]->long_click.handler = mClickHandler;
#endif //SCREEN_CAPTURE_ENABLED
}


static bool s_pleaseWaitOnTop = false;
void PleaseWaitCreate()
{
	window_init	(&s_window, "PleaseWait");

	text_layer_init(&s_title, GRect(10,30,134, 60));
	text_layer_init(&s_body, GRect(10,60,134, 60));
	text_layer_init(&s_debug, GRect(10,100,134, 80));
	text_layer_init(&s_version, GRect(10,135,134, 19));

 	text_layer_set_text	(&s_title, "Please Wait");
    text_layer_set_font(&s_title, fonts_get_system_font(FONT_KEY_GOTHIC_28));
    text_layer_set_overflow_mode(&s_title, GTextOverflowModeWordWrap );

    text_layer_set_font(&s_body, fonts_get_system_font(FONT_KEY_GOTHIC_28));
    text_layer_set_overflow_mode(&s_body, GTextOverflowModeWordWrap );

    text_layer_set_font(&s_debug, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    text_layer_set_overflow_mode(&s_debug, GTextOverflowModeWordWrap );

    text_layer_set_font(&s_version, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    text_layer_set_overflow_mode(&s_version, GTextOverflowModeWordWrap );

	layer_add_child(&s_window.layer, &s_title.layer);
	layer_add_child(&s_window.layer, &s_body.layer);
	layer_add_child(&s_window.layer, &s_debug.layer);
	layer_add_child(&s_window.layer, &s_version.layer);

 	text_layer_set_text	(&s_version, VERSION_STRING);
    s_pleaseWaitOnTop = false;


    window_set_click_config_provider(&s_window,mClickConfigProvider);

}

bool PleaseWaitActive()
{
	return s_pleaseWaitOnTop;
}

void PleaseWaitShow(const char const * body)
{
 	text_layer_set_text	(&s_body, body);
 	window_stack_remove(&s_window, false);
 	window_stack_push(&s_window, true);
}

void PleaseWaitDebug(const char const *debug)
{
 	text_layer_set_text	(&s_debug, debug);
}

void PleaseWaitHide()
{
	s_pleaseWaitOnTop = false;
 	window_stack_pop(true);
}
