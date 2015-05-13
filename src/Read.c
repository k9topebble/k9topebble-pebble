/*
 * Read.c
 *
 *  Created on: 1 Jul 2013
 *      Author: andrew
 */

#include "pebble_app.h"
#include "pebble_app_info.h"
#include "pebble_fonts.h"
#include "MessageData.h"
#include "Read.h"
#include "PleaseWait.h"
#include "logger.h"
#include "ScreenCapture.h"

static Window s_window;
static char s_uri[MAX_URL_LENGTH];
static char s_sender_txt[MAX_SENDER_LENGTH];
static char s_subject_txt[MAX_SUBJECT_LENGTH];

static bool s_waitingForData;
static bool s_gotBody;

static TextLayer s_sender;
static TextLayer s_subject;
static TextLayer s_body;

static InverterLayer s_inverter;
static GRect scroll_rect = {{0,0},{144,168}};
static GRect body_rect = {{0,0},{144,1600}};

static ScrollLayer s_scroll;

static void handle_disappear(Window *window);
static void readUpdateDisplay();



#ifdef SCREEN_CAPTURE_ENABLED
static void mClickHandler(ClickRecognizerRef recognizer, void *context)
{
	pbl_capture_send();
}

static void mClickConfigProvider(ClickConfig **array_of_ptrs_to_click_configs_to_setup, void *context)
{

	array_of_ptrs_to_click_configs_to_setup[BUTTON_ID_SELECT]->long_click.handler = mClickHandler;
}
#endif //SCREEN_CAPTURE_ENABLED


void ReadCreate()
{
	s_waitingForData = false;
	s_gotBody = false;
}

void ReadShow(const char const* uri)
{
	s_waitingForData = true;
	s_gotBody = false;

    if (uri)
    {
    	strncpy(s_uri, uri, MAX_URL_LENGTH);
    	//LOGGER_S("Show body");
    }
    else
    {
    	return;
    }

	// at this point there may be no data to work with, if so, show 'please wait' screen.
	if (!md_requestBody(s_uri))
	{
		LOGGER_S("Email loading");
		s_waitingForData = true;
		s_gotBody = false;
	}
	else
	{
		LOGGER_S("Email already loaded");
		s_gotBody = true;
	}

 	int pos = -1;
 	pos = md_index(s_uri);
	strncpy(s_sender_txt, md_sender(pos), MAX_SENDER_LENGTH);
	strncpy(s_subject_txt, md_subject(pos), MAX_SUBJECT_LENGTH);


	window_init	(&s_window, "email window");
	window_set_fullscreen(&s_window, true);

    window_set_window_handlers(&s_window, (WindowHandlers){
        .disappear = (WindowHandler)handle_disappear
    });
 	scroll_layer_init(&s_scroll,scroll_rect);

 	GContext * 	ctx = app_get_current_graphics_context();


    GFont fSender;
    GFont fSubject;
    GFont fBody;
    switch (md_inboxTextSize())
    {
    case eSize_Small:
    	fSender = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
    	fSubject = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
    	fBody = fonts_get_system_font(FONT_KEY_GOTHIC_18);
    	break;
    case eSize_Large:
    	fSender = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
    	fSubject = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
    	fBody = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
    	break;
    case eSize_Regular:
    default:
    	fSender = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
    	fSubject = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
    	fBody = fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21);
    	break;
    }


 	text_layer_init	(&s_sender,GRect(0,0, 144, 200));
    text_layer_set_font(&s_sender, fSender);
    text_layer_set_overflow_mode(&s_sender, GTextOverflowModeTrailingEllipsis );
	GSize senderSize = text_layer_get_max_used_size	(ctx, &s_sender);

 	text_layer_init	(&s_subject,GRect(0,0, 144, 200));
    text_layer_set_font(&s_subject, fSubject);
    text_layer_set_overflow_mode(&s_sender, GTextOverflowModeTrailingEllipsis );
    GSize subjectSize = text_layer_get_max_used_size(ctx, &s_subject);

    inverter_layer_init(&s_inverter, GRect(0,0, 144, subjectSize.h + 5));

 	text_layer_init	(&s_body,GRect(0,0, 144, 1600));
    text_layer_set_font(&s_body, fBody);
    text_layer_set_overflow_mode(&s_body, GTextOverflowModeTrailingEllipsis );

 	scroll_layer_set_click_config_onto_window(&s_scroll, &s_window);

 	scroll_layer_add_child	(&s_scroll, &s_sender.layer);
 	scroll_layer_add_child	(&s_scroll, &s_subject.layer);
 	scroll_layer_add_child	(&s_scroll, &s_inverter.layer);
 	scroll_layer_add_child	(&s_scroll, &s_body.layer);

	layer_add_child(&s_window.layer, &s_scroll.layer);


	readUpdateDisplay();
 	window_stack_push(&s_window, true);
#ifdef SCREEN_CAPTURE_ENABLED
    window_set_click_config_provider(&s_window,mClickConfigProvider);
#endif //SCREEN_CAPTURE_ENABLED
}

void readUpdateDisplay()
{
 	GContext * 	ctx = app_get_current_graphics_context();

	text_layer_set_size	(&s_body,body_rect.size);

 	if (s_gotBody)
 	{
 		text_layer_set_text	(&s_body, md_body());
 		light_enable_interaction();
 	}

  	text_layer_set_text(&s_sender, s_sender_txt);
 	text_layer_set_text(&s_subject, s_subject_txt);

 	if (s_gotBody)
 	{
 		text_layer_set_text(&s_body, md_body());
 	}
 	else
 	{
 		text_layer_set_text(&s_body, "Please Wait, loading");
 	}

 	GSize senderSize =  text_layer_get_max_used_size(ctx, &s_sender);
    GSize subjectSize = text_layer_get_max_used_size(ctx, &s_subject);
    GSize bodySize =    text_layer_get_max_used_size(ctx, &s_body);

    int senderLimit;
    int subjectLimit;
    int subjectOverlap;

    switch (md_inboxTextSize())
    {
    case eSize_Small:
        senderLimit = 18*2;
        subjectLimit = 14*3;
        subjectOverlap = 5;
    	break;
    case eSize_Large:
        senderLimit = 28*2;
        subjectLimit = 14*3;
        subjectOverlap = 7;
    	break;
    case eSize_Regular:
    default:
        senderLimit = 24*2;
        subjectLimit = 18*3;
        subjectOverlap = 5;
        break;
    }

    if (senderSize.h> senderLimit)
    {
    	senderSize.h = senderLimit;
    }

    if (subjectSize.h> subjectLimit)
    {
    	subjectSize.h = subjectLimit;
    }

    layer_set_frame(&s_sender.layer,  GRect(0,0, 144, 200));
	layer_set_frame(&s_subject.layer, GRect(0,senderSize.h +subjectOverlap, 144, 200));
    layer_set_frame(&s_body.layer,    GRect(0,senderSize.h+subjectSize.h + subjectOverlap*2, 144, 1600));
    layer_set_frame(&s_inverter.layer,GRect(0,senderSize.h +subjectOverlap, 144, subjectSize.h + subjectOverlap));

//    text_layer_set_size	(&s_sender, senderSize);
//  	text_layer_set_size	(&s_subject,subjectSize);
//  	text_layer_set_size	(&s_body,bodySize);

 	GSize scrollSize = {0,0};
 	scrollSize.w += senderSize.w;
 	scrollSize.h += senderSize.h;
 	scrollSize.h += subjectSize.h;
 	scrollSize.h += bodySize.h + 24;

 	scroll_layer_set_content_size(&s_scroll, scrollSize);
}


void ReadRefresh()
{
	LOGGER_S("Loaded body data");
 	GContext * 	ctx = app_get_current_graphics_context();

	//if (!md_bodyLoaded())
	/*{
		LOGGER_S("Email loading");
		s_waitingForData = true;
		s_gotBody = false;
		return;
	}
	else
	{
		LOGGER_S("Email already loaded");
		s_gotBody = true;
	}*/
 	s_gotBody = true;
	readUpdateDisplay();
}
void ReadHide()
{
}
void ReadDestroy()
{

}
void handle_disappear(Window *window)
{
	window_deinit(window);
}
