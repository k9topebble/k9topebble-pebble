
#include "pebble_app.h"
#include "pebble_app_info.h"
#include "MessageData.h"
#include "Inbox.h"
#include "Action.h"
#include "pebble_fonts.h"
#include "PleaseWait.h"
#include "Read.h"
#include "logger.h"
#include "ScreenCapture.h"
#include "AreYouSure.h"


static void m_MenuLayerDrawHeaderCallback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context);
static void m_MenuLayerDrawRowCallback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context);
static int16_t m_MenuLayerGetCellHeightCallback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static uint16_t m_MenuLayerGetNumberOfRowsInSectionsCallback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static void m_MenuLayerSelectCallback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void m_MenuLayerLongSelectCallback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);

static MenuLayer s_menu_layer;
static Window s_window;
static char s_uri[256];
static bool s_waitingForData = false;

static MenuLayerCallbacks s_callbacks = {
	.get_num_sections = 0,
	.get_num_rows = m_MenuLayerGetNumberOfRowsInSectionsCallback,
	.draw_header = m_MenuLayerDrawHeaderCallback,
	.draw_row = m_MenuLayerDrawRowCallback,
	.get_cell_height = m_MenuLayerGetCellHeightCallback,
	.select_click = m_MenuLayerSelectCallback,
	.select_long_click = m_MenuLayerLongSelectCallback
};

void m_MenuLayerDrawHeaderCallback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context)
{
	menu_cell_basic_header_draw	(ctx, cell_layer, "Emails");
}
void m_MenuLayerDrawRowCallback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context)
{
    graphics_context_set_text_color(ctx, GColorBlack);

    if (md_unread(cell_index->row))
    {
    	graphics_text_draw(ctx, "U", fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(0,10,11,14), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
    }
    if (md_isNew(cell_index->row))
    {
    	graphics_text_draw(ctx, "N", fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(0,45-18,11,14), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
    }

    GFont fSender;
    int fSenderHeight;
    GFont fSubject;
    int fSubjectHeight;
    switch (md_inboxTextSize())
    {
    case eSize_Small:
    	fSender = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
    	fSubject = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
        fSenderHeight = 18;
        fSubjectHeight = 16;
    	break;
    case eSize_Large:
    	fSender = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
    	fSubject = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
        fSenderHeight = 28;
        fSubjectHeight = 26;
    	break;
    case eSize_Regular:
    default:
    	fSender = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
    	fSubject = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
        fSenderHeight = 24;
        fSubjectHeight = 20;
    	break;
    }

	graphics_text_draw(ctx, md_sender(cell_index->row),
			fSender,
			GRect(12,0,cell_layer->bounds.size.w - 12,fSenderHeight),
			GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
	graphics_text_draw(ctx, md_subject(cell_index->row),
			fSubject,
			GRect(12,fSenderHeight + 1,cell_layer->bounds.size.w - 12,fSubjectHeight),
			GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);

    if (md_isDeleted(cell_index->row))
    {
    	int SenlinePos = fSenderHeight + fSenderHeight/2;
    	int SublinePos = (fSubjectHeight/3) *2;
		graphics_context_set_stroke_color(ctx, GColorBlack);
		graphics_draw_line(ctx, GPoint(12, SenlinePos), GPoint(cell_layer->bounds.size.w - 12, SenlinePos));
		graphics_draw_line(ctx, GPoint(12, SenlinePos + 1), GPoint(cell_layer->bounds.size.w - 12, SenlinePos + 1));
		graphics_draw_line(ctx, GPoint(12, SublinePos), GPoint(cell_layer->bounds.size.w - 12, SublinePos));
		graphics_draw_line(ctx, GPoint(12, SublinePos+1), GPoint(cell_layer->bounds.size.w - 12, SublinePos + 1));
    }
	//menu_cell_basic_draw(ctx, cell_layer, md_sender(cell_index->row), md_subject(cell_index->row),
	//		&N_IMG.bmp // icon, tod
	//	);
/*	menu_cell_basic_draw(ctx, cell_layer,
			"Foo",
			"Bar",
			0 // icon, todo
		);
*/
}

int16_t m_MenuLayerGetCellHeightCallback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context)
{
    switch (md_inboxTextSize())
    {
    case eSize_Small:
        return 35;
    case eSize_Large:
    	return 55;
    case eSize_Regular:
    default:
        return 45;
    }
}
uint16_t m_MenuLayerGetNumberOfRowsInSectionsCallback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context)
{
	return md_getMessageCount();
}
void m_MenuLayerSelectCallback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context)
{
	if (!md_isDeleted(cell_index->row))
	{
		//ActionShow(md_uuid(cell_index->row));
		ReadShow(md_uuid(cell_index->row));
	}
}

static char s_deleteuuid[MAX_URL_LENGTH];

void deleteCB(void* data)
{
	if (AreYouSureConfirm())
	{
		md_requestDelete(s_deleteuuid);
		InboxRefresh();
	}
}

void m_MenuLayerLongSelectCallback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context)
{
	if (!md_isDeleted(cell_index->row))
	{
		strncpy(s_deleteuuid, md_uuid(cell_index->row), MAX_URL_LENGTH);
		s_deleteuuid[MAX_URL_LENGTH -1] = 0;
		AreYouSureShow(md_subject(cell_index->row), deleteCB);
	}
}

static bool s_visible;
static void s_appear(struct Window *window)
{
	s_visible = true;
	menu_layer_reload_data(&s_menu_layer);
	K9_APP_LOG(APP_LOG_LEVEL_DEBUG, "inbox show");
}

static void s_disapear(struct Window *window)
{
	s_visible = false;
	K9_APP_LOG(APP_LOG_LEVEL_DEBUG, "inbox hide");
}

void InboxCreate()
{
	window_init	(&s_window, "email window");
	window_set_fullscreen(&s_window, true);
	menu_layer_init	(&s_menu_layer, s_window.layer.frame);
	layer_add_child(&s_window.layer, &s_menu_layer.scroll_layer.layer);
	s_waitingForData = false;
	menu_layer_set_callbacks(&s_menu_layer,  0, s_callbacks);

	menu_layer_set_click_config_onto_window	(&s_menu_layer, &s_window);

	WindowHandlers wh =  {.appear = s_appear, .disappear = s_disapear};

	window_set_window_handlers(&s_window, wh);

}

void InboxShow(const char const* uri)
{
	if (uri)
    {
    	strncpy(s_uri, uri, 256);
    }

	// at this point there may be no data to work with, if so, show 'please wait' screen.
	if (!md_loaded())
	{
		PleaseWaitShow("Inbox loading");
		s_waitingForData = true;
		return;
	}

 	int pos = -1;
 	if (uri)
 	{
 		pos = md_index(uri);
 	}

 	if (pos >= 0)
 	{
 		MenuIndex index;
 		index.row = pos;
 		index.section = 0;
 		menu_layer_set_selected_index(&s_menu_layer, index, MenuRowAlignCenter, false);
 	}
    window_stack_push(&s_window, true);
}

void InboxRefresh()
{
	if (s_visible)
	{
		menu_layer_reload_data(&s_menu_layer);
		K9_APP_LOG(APP_LOG_LEVEL_DEBUG, "reloaded menu data");
	}
	if (s_waitingForData)
	{
		s_waitingForData = false;
		PleaseWaitHide();
		InboxShow(0);
		light_enable_interaction();
	}
}

void InboxHide()
{
	//window_stack_remove(&s_window);
}
void InboxDestroy()
{
	window_deinit(&s_window);
}

