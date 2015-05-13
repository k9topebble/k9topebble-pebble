
#include "pebble_app.h"
#include "pebble_app_info.h"
#include "stdbool.h"
#include "Action.h"
#include "Inbox.h"
#include "Read.h"
#include "logger.h"

#if 0

static void actionDelete(int index, void *context);
static void actionRead(int index, void *context);
static void actionInbox(int index, void *context);


static SimpleMenuLayer s_menu;
static Window s_window;
static SimpleMenuLayer s_menuNoURI;
static Window s_windowNoURI;
static char s_uri[256];
static bool s_init = false;


static SimpleMenuItem s_menuItems[] = {
		{.callback = actionInbox, .title = "Inbox"},
		{.callback = actionDelete, .title = "Delete"},
		{.callback = actionRead, .title = "Read"},

};


static SimpleMenuSection s_actionMenu[]= {{
		.items=s_menuItems,
		.num_items=3,
		.title="Actions"
}};

static SimpleMenuItem s_menuItemsNoURI[] = {
		{.callback = actionInbox, .title = "Inbox"},
};


static SimpleMenuSection s_actionMenuNoURI[]= {{
		.items=s_menuItemsNoURI,
		.num_items=ARRAY_LENGTH(s_menuItemsNoURI),
		.title="Actions"
}};


void ActionCreate()
{
	window_init	(&s_window, "action window");

	simple_menu_layer_init(&s_menu, s_window.layer.frame, &s_window, s_actionMenu, 1, 0);

	window_init	(&s_windowNoURI, "action window");

	simple_menu_layer_init(&s_menuNoURI, s_windowNoURI.layer.frame, &s_windowNoURI,	s_actionMenuNoURI, 1, 0);

	layer_add_child(&s_window.layer, &s_menu.menu.scroll_layer.layer);
	layer_add_child(&s_windowNoURI.layer, &s_menuNoURI.menu.scroll_layer.layer);
}

void ActionShow(const char* uri)
{
    if (uri)
    {
    	 strncpy(s_uri, uri, 256);
         window_stack_push(&s_window, true);

         simple_menu_layer_set_selected_index(&s_menu, 1, false);
    }
    else
    {
    	s_uri[0] = 0;
        window_stack_push(&s_windowNoURI, true);
    }
}

void ActionHide()
{
	//window_stack_remove(&s_window);
}
void ActionDestroy()
{
	window_deinit(&s_window);
	window_deinit(&s_windowNoURI);
	s_init = false;
}

void actionDelete(int index, void *context)
{
	// issue delete request then launch inbox
	InboxShow(s_uri);
}
void actionRead(int index, void *context)
{
	LOGGER_S("actonRead");
	ReadShow(s_uri);
}
void actionInbox(int index, void *context)
{
	InboxShow(s_uri);
	ActionHide();
}

#endif


