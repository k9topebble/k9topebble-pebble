/*
 * MessageData.h
 *
 *  Created on: 30 Jun 2013
 *      Author: andrew
 */
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include "pebble_os.h"
#include "defines.h"

#ifndef MESSAGE_DATA_H
#define MESSAGE_DATA_H
int md_getMessageCount();
const char const * md_sender(int index);
//const char const * md_date(int index);
const char const * md_subject(int index);
const char const * md_body();
const char const * md_uuid(int index);
int md_index(const char const * uuid);
bool md_unread(int index);
bool md_hasAttachment(int index);
bool md_isNew(int index);
bool md_isDeleted(int index);

bool md_activate();
bool md_deactivate();
bool md_loaded();
bool md_requestBody(const char const * uri);
bool md_bodyLoaded();
bool md_requestDelete(const char const * uri);

void my_out_sent_handler(DictionaryIterator *sent, void *context);
void my_out_fail_handler(DictionaryIterator *failed, AppMessageResult reason, void *context);
void my_in_rcv_handler(DictionaryIterator *received, void *context);
void my_in_drp_handler(void *context, AppMessageResult reason);

DisplayTextSize md_inboxTextSize();
DisplayTextSize md_bodyTextSize();


#endif //MESSAGE_DATA_H
