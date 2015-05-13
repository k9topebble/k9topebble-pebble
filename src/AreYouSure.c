/*
 * AreYouSure.c
 *
 *  Created on: 25 Jul 2013
 *      Author: andrew
 */


/*
 * show 'Delete <subject>?
 * Select = yes, back = no.
 *
 */

#include "pebble_app.h"
#include "pebble_app_info.h"
#include "pebble_fonts.h"
#include "AreYouSure.h"
#include "stdbool.h"
#include "logger.h"
#include "defines.h"
#include "ScreenCapture.h"

static Window s_window;
static TextLayer s_title;
static TextLayer s_body;
static TextLayer s_instruction;
static AYS_CALLBACK s_cb;

static bool s_AreYouSureConfirm = false;
static void mClickHandler(ClickRecognizerRef recognizer, void *context)
{
	s_AreYouSureConfirm = true;

	s_cb(0);
	AreYouSureHide();
}

static void mClickConfigProvider(ClickConfig **array_of_ptrs_to_click_configs_to_setup, void *context)
{
	array_of_ptrs_to_click_configs_to_setup[BUTTON_ID_SELECT]->long_click.handler = mClickHandler;
	array_of_ptrs_to_click_configs_to_setup[BUTTON_ID_SELECT]->click.handler = mClickHandler;
}

void AreYouSureCreate()
{
	window_init	(&s_window, "Are you sure?");

	text_layer_init(&s_title, GRect(10,10,134, 30));
	text_layer_init(&s_body, GRect(10,40,134, 60));
	text_layer_init(&s_instruction, GRect(10,100,134, 60));

 	text_layer_set_text	(&s_title, "Delete email?");
    text_layer_set_font(&s_title, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_overflow_mode(&s_title, GTextOverflowModeWordWrap );

    text_layer_set_font(&s_body, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_overflow_mode(&s_body, GTextOverflowModeWordWrap );

    text_layer_set_text	(&s_instruction, "Select to confirm, back to cancel");
    text_layer_set_font(&s_instruction, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_overflow_mode(&s_instruction, GTextOverflowModeWordWrap );

	layer_add_child(&s_window.layer, &s_title.layer);
	layer_add_child(&s_window.layer, &s_body.layer);
	layer_add_child(&s_window.layer, &s_instruction.layer);

	window_set_click_config_provider(&s_window,mClickConfigProvider);
}

bool AreYouSureConfirm()
{
	return s_AreYouSureConfirm;
}

void AreYouSureShow(const char const *body, AYS_CALLBACK cb)
{
	s_AreYouSureConfirm = false;
	text_layer_set_text	(&s_body, body);
 	window_stack_remove(&s_window, false);
 	window_stack_push(&s_window, true);
 	s_cb = cb;
}

void AreYouSureHide()
{
 	window_stack_pop(true);
}
