
#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "Inbox.h"
#include "Action.h"
#include "Read.h"
#include "AreYouSure.h"
#include "MessageData.h"
#include "PleaseWait.h"
#include "messageQ.h"
#include "logger.h"
#include "defines.h"


#define MY_UUID { 0xc4, 0x44, 0x7e, 0xb9, 0x5f, 0x01, 0x4f, 0x3f, 0x92, 0xb8, 0xd2, 0xea, 0x8f, 0x20, 0x98, 0x30}
PBL_APP_INFO(MY_UUID, "K9ToPebble", "edisms.org",MAJOR_VERSION, MINOR_VERSION, RESOURCE_ID_TITLE, APP_INFO_STANDARD_APP);

static AppTimerHandle s_timer;

#if 1
Window window;

TextLayer textLayer;


void up_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
	(void)recognizer;
	(void)window;

}


void down_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
	(void)recognizer;
	(void)window;

}


void select_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
	(void)recognizer;
	(void)window;

	text_layer_set_text(&textLayer, "Start!");
	InboxShow(0);
}


void select_long_click_handler(ClickRecognizerRef recognizer, Window *window) {
	(void)recognizer;
	(void)window;

}


// This usually won't need to be modified

void click_config_provider(ClickConfig **config, Window *window) {
	(void)window;

	config[BUTTON_ID_SELECT]->click.handler = (ClickHandler) select_single_click_handler;

	config[BUTTON_ID_SELECT]->long_click.handler = (ClickHandler) select_long_click_handler;

	config[BUTTON_ID_UP]->click.handler = (ClickHandler) up_single_click_handler;
	config[BUTTON_ID_UP]->click.repeat_interval_ms = 100;

	config[BUTTON_ID_DOWN]->click.handler = (ClickHandler) down_single_click_handler;
	config[BUTTON_ID_DOWN]->click.repeat_interval_ms = 100;
}
#endif

static void m_PebbleAppTimerHandler(AppContextRef app_ctx, AppTimerHandle handle, uint32_t cookie)
{
	mq_pop();
	s_timer = app_timer_send_event(app_ctx, 200,1);
	if (s_timer == 0)
	{
		K9_APP_LOG(APP_LOG_LEVEL_DEBUG, "Invalid timer handle");
	}
}


// Standard app initialisation

void handle_init(AppContextRef ctx) {
	(void)ctx;
	mq_create();
//	ActionCreate();
	AreYouSureCreate();
	ReadCreate();
	InboxCreate();
	PleaseWaitCreate();
	md_activate();
	InboxShow(0);
	s_timer = app_timer_send_event(ctx, 200,1);
	if (s_timer == 0)
	{
		K9_APP_LOG(APP_LOG_LEVEL_DEBUG, "Invalid timer handle");
	}
}

void handle_deinit(AppContextRef ctx) {
	(void)ctx;
	md_deactivate();
}

void pbl_main(void *params) {
	static PebbleAppHandlers s_handlers = {
			.init_handler = &handle_init,
			.deinit_handler = &handle_deinit,
			.timer_handler = m_PebbleAppTimerHandler,
			.messaging_info = {
					.buffer_sizes = {
							.inbound = 256, // inbound buffer size in bytes
							.outbound = MAX_OUT_MESSAGE_SIZE, // outbound buffer size in bytes
					},
					.default_callbacks.callbacks = {
							.out_sent = my_out_sent_handler,
							.out_failed = my_out_fail_handler,
							.in_received = my_in_rcv_handler,
							.in_dropped = my_in_drp_handler,
					},
			},
	};
	app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
	app_event_loop(params, &s_handlers);
}

