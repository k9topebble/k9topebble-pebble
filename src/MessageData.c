/*
 * MessageData.c
 *
 *  Created on: 30 Jun 2013
 *      Author: andrew
 */
#include "MessageData.h"
#include "Inbox.h"
#include "Read.h"
#include "PleaseWait.h"
#include "defines.h"

#include "messageQ.h"

typedef struct _messageData
{
	char url[MAX_URL_LENGTH];
	char sender[MAX_SENDER_LENGTH];
	char subject[MAX_SUBJECT_LENGTH];
	bool unread;
	bool deleted;
} messageData;

static messageData s_messageData[MAX_MESSAGE_COUNT];
static bool _sendMessage(MessageTypesCommands mt, const char const * uri, MQ_CALLBACK);

static DictionaryIterator s_body;

static uint8_t s_bodyBuffer[BODY_BUFFER_SIZE];
static uint16_t s_messageKeys[MAX_MESSAGE_COUNT];
static char s_bodyExtract[MAX_BODY_SIZE];
static char s_bodyuri[MAX_URL_LENGTH];
static int16_t s_messageCount;
static bool s_hasLoaded;
static bool s_bodyFailed;
static int s_bodyTextSize;
static int s_inboxTextSize;


static int error_count;

const char* msgError(AppMessageResult reason)
{
	switch (reason)
	{
	default:
	case APP_MSG_SEND_TIMEOUT:
		return "Failed to contact android app.  Please try again";
		break;
	case APP_MSG_SEND_REJECTED:
		return "Message rejected";
		break;
	case APP_MSG_NOT_CONNECTED:
		return "Not connected to host!";
		break;
	case APP_MSG_APP_NOT_RUNNING:
		return "App not running";
		break;
	case APP_MSG_INVALID_ARGS:
		return "Invalid arguments";
		break;
	case APP_MSG_BUSY:
		return "Android app not responding";
		break;
	case APP_MSG_BUFFER_OVERFLOW:
		return "Protocol error, please contact support";
		break;
	case APP_MSG_ALREADY_RELEASED:
		return "Protocol error, please contact support";
		break;
	case APP_MSG_CALLBACK_ALREADY_REGISTERED:
		return "Weird error 1";
		break;
	case APP_MSG_CALLBACK_NOT_REGISTERED:
		return "Weird error 2";
		break;
	}
	return "Weird error 3";
}

void activate_CALLBACK(DictionaryIterator*source, AppMessageResult reason, void*ignore)
{
	if (reason != APP_MSG_OK)
	{
		if (error_count < 3)
		{
			// resend
			mq_post_cb(source, activate_CALLBACK, 0);
			error_count++;
			return;
		}
		PleaseWaitDebug(msgError(reason));
	}
}

void body_CALLBACK(DictionaryIterator*source, AppMessageResult reason, void*ignore)
{
	if (reason != APP_MSG_OK)
	{
		if (error_count < 3)
		{
			// resend
			mq_post_cb(source, body_CALLBACK, 0);
			error_count++;

			return;
		}
		strcpy(s_bodyExtract, msgError(reason));
		s_bodyFailed = true;
		ReadRefresh();
	}
}

bool md_activate()
{
	//dict_write_begin(&s_inbox, s_inboxBuffer, INBOX_BUFFER_SIZE);
	//dict_write_end(&s_inbox);
	dict_write_begin(&s_body, s_bodyBuffer, BODY_BUFFER_SIZE);
	dict_write_end(&s_body);
	s_messageCount = 0;
	s_hasLoaded = false;

	memset(s_messageKeys, 0, sizeof(s_messageKeys));
    memset(s_messageData, 0, sizeof(s_messageData));

    K9_APP_LOG(APP_LOG_LEVEL_DEBUG, "SendActivate");

	DictionaryResult ret;
	DictionaryIterator command;
	uint8_t buff[MAX_OUT_MESSAGE_SIZE];

	DictionaryResult res = dict_write_begin(&command, buff, MAX_OUT_MESSAGE_SIZE);
	if (res != DICT_OK)
	{
		return false;
	}
	uint8_t msg =  eMT_RequestStart;
	ret = dict_write_int(&command, KEY_COMMAND, &msg, 1, false);
	if (DICT_OK != ret)
	{
		return false;
	}
	msg =  PROTOCOL_VERSION;
	ret = dict_write_int(&command, KEY_PROTOCOL_VERSION, &msg, 1, false);
	if (DICT_OK != ret)
	{
		return false;
	}

	dict_write_end(&command);

	//mq_post(&command);

	error_count = 0;

	mq_post_cb(&command, activate_CALLBACK, 0);

	s_bodyTextSize = 0;
	s_inboxTextSize = 0;

	return ret;
}

bool md_deactivate()
{
	K9_APP_LOG(APP_LOG_LEVEL_DEBUG, "SendDeactivate");
	return _sendMessage(eMT_RequestStop, 0, 0);
}

bool md_loaded()
{
	return s_hasLoaded;
}

#if 0
// iterate over the database to
static void _processKeys()
{
    //iterate over the db looking for emails,
	//where a base (UUID) key is found, store the key value in the key map.

	Tuple*tuple;
	tuple =dict_read_first(&s_inbox);
	while(tuple != NULL)
	{
		if (tuple->key >= 10)
		{
			if ((tuple->key % 10 == 0)  && (s_messageCount < MAX_MESSAGE_COUNT))
			{
				// look for a slot
				int i;
				for (i = 0; i < MAX_MESSAGE_COUNT; i++)
				{
					if (s_messageKeys[i] == tuple->key)
					{
						// this one is already know, skip
						break;
					}
					else if (s_messageKeys[i] == 0)
					{
						// found a new entry
						s_messageKeys[s_messageCount++] = tuple->key;
						break;
					}
				}
			}
		}
		tuple = dict_read_next(&s_inbox);
	}
	LOGGER_SI("_processKeys found", s_messageCount);
}
#endif

bool _isBodyComplete(DictionaryIterator *body)
{
	// key 1 contains the overall length.
	// then iterate over the following keys to get the amount that's received.
	// if it's complete, copy into the body buffer

	int bodySize = 0;
	Tuple *size = dict_find(body, 1);
	if (size)
	{
		if (size->type == TUPLE_UINT && size->length == 2)
		{
			bodySize = size->value->uint16;
		}
		else
		{
			K9_APP_LOG(APP_LOG_LEVEL_DEBUG, "bad command type %d", size->type);
			return false;
		}
	}
	else
	{
		K9_APP_LOG(APP_LOG_LEVEL_DEBUG, "no size");
		return false;
	}
	//LOGGER_SI("target size", bodySize);

	// ok, got something to work with, is it complete
	{
		Tuple *content = dict_find(body, 10);
		int pos = 11;
		uint16_t gotSize = 0;
		uint16_t offset = 0;
		while (content)
		{
			gotSize += content->length;
			//LOGGER_SI("chunk size", content->length);
			content = dict_find(body, pos++);
		}

		if (gotSize < bodySize)
		{
			//LOGGER_SII("got body/size", gotSize, bodySize);

			return false;
		}

		// ok, we've got enough data, lets put it somewhere safe
		content = dict_find(body, 10);
		pos = 11;
		while (content)
		{
			memcpy(s_bodyExtract + offset, content->value->cstring, content->length );
			offset += content->length - 1; // -1 so the next block overwrites the null at the end of this block
			content = dict_find(body, pos++);
		}

		s_bodyExtract[MAX_BODY_SIZE - 1] = 0; // ensure it's null terminated
	}
	K9_APP_LOG(APP_LOG_LEVEL_DEBUG, "Body ok");
	return true;
}

bool md_requestBody(const char const * uri)
{
    if (uri)
    {
    	if (strncmp(uri, s_bodyuri, MAX_URL_LENGTH) == 0)
    	{
    		if (_isBodyComplete(&s_body))
    		{
    			K9_APP_LOG(APP_LOG_LEVEL_DEBUG, "body ok");
    			return true;
    		}
    	}
    	memset(s_bodyuri, 0, MAX_URL_LENGTH);
    	strncpy(s_bodyuri, uri, MAX_URL_LENGTH);
    	s_bodyuri[MAX_URL_LENGTH-1] = 0;
    }

    app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
    //reset the body
	dict_write_begin(&s_body, s_bodyBuffer, BODY_BUFFER_SIZE);
	dict_write_end(&s_body);

	//LOGGER_SS("request body 1 ", s_bodyuri);
	K9_APP_LOG(APP_LOG_LEVEL_DEBUG, "request body 2 %s", uri);

	error_count = 0;
	s_bodyFailed = false;
    _sendMessage(eMT_RequestBody, s_bodyuri, body_CALLBACK);

	return false;
}

bool md_requestDelete(const char const * uri)
{
    _sendMessage(eMT_RequestDelete, uri, 0);
    int index = md_index(uri);
    if (index >= 0)
    {
    	s_messageData[index].deleted = true;
    }

	return true;
}

bool md_bodyLoaded()
{
	if (s_bodyFailed)
	{
		return true;
	}
	if (_isBodyComplete(&s_body))
	{
		K9_APP_LOG(APP_LOG_LEVEL_DEBUG, "body ok");
		return true;
	}
	return false;
}


bool _sendMessage(MessageTypesCommands mt, const char const * uri, MQ_CALLBACK cb)
{
	DictionaryResult ret;
	DictionaryIterator command;
	uint8_t buff[MAX_OUT_MESSAGE_SIZE];

	DictionaryResult res = dict_write_begin(&command, buff, MAX_OUT_MESSAGE_SIZE);
	if (res != DICT_OK)
	{
		//PleaseWaitDebug("MQ can't write");
		return false;
	}
	uint8_t msg =  mt;
	ret = dict_write_int(&command, 0, &msg, 1, false);
	if (DICT_OK != ret)
	{
		return false;
	}
	if (uri)
	{
	    ret = dict_write_cstring(&command, KEY_URL,uri);
		if (DICT_OK != ret)
		{
			return false;
		}
	}

	dict_write_end(&command);

	mq_post_cb(&command, cb, 0);
	return true;
}

void md_BodyDictionaryUpdate(const uint32_t key, const Tuple *new_tuple, const Tuple *old_tuple, void *context)
{
	// is the read screen open?  If so trigger a refresh
}


bool keyComplete(int key, DictionaryIterator *received)
{
	int baseKey = (key /10) *10;
	Tuple *uuid = dict_find(received, baseKey + KEY_UUID);
	Tuple *sender = dict_find(received, baseKey + KEY_SENDER);
	Tuple *subject = dict_find(received, baseKey + KEY_SUBJECT);
	Tuple *unread = dict_find(received, baseKey + KEY_UNREAD);

	bool res = uuid && sender && subject && unread;
	/*if (res)
	{
		LOGGER_SI("Key complete ", baseKey);
	}*/
	return res;
}

void markMissing(int last, int expected)
{
	DictionaryResult ret;
	DictionaryIterator command;
	uint8_t buff[MAX_OUT_MESSAGE_SIZE];

	DictionaryResult res = dict_write_begin(&command, buff, MAX_OUT_MESSAGE_SIZE);
	if (res != DICT_OK)
	{
		//PleaseWaitDebug("MQ can't write");
		return;
	}
	uint8_t msg =  eMT_RequestMissing;
	ret = dict_write_int(&command, 0, &msg, 1, false);
	if (DICT_OK != ret)
	{
		return;
	}

	dict_write_int(&command, KEY_EXPECTED, &expected, 4, false);
	dict_write_int(&command, KEY_LAST,&last, 4, false);

	dict_write_end(&command);

	mq_post(&command);
	return;
}

void sendAck(int msgID)
{
	DictionaryResult ret;
	DictionaryIterator command;
	uint8_t buff[MAX_OUT_MESSAGE_SIZE];

	DictionaryResult res = dict_write_begin(&command, buff, MAX_OUT_MESSAGE_SIZE);
	if (res != DICT_OK)
	{
		//PleaseWaitDebug("MQ can't write");
		return;
	}
	uint8_t msg =  eMT_ConfirmTag;
	ret = dict_write_int(&command, 0, &msg, 1, false);
	if (DICT_OK != ret)
	{
		return;
	}

	dict_write_int(&command, KEY_LAST, &msgID, 4, false);

	dict_write_end(&command);

	//mq_post(&command);
	return;
}


void addDataToMsgs(Tuple * tuple)
{
	if (tuple->key < KEY_START)
	{
		return;
	}
	// extract message id
	int message_id = (tuple->key / 100);
	int offset = tuple->key - (message_id * 100);
	message_id--;

	//LOGGER_SIII("msg key, id,  and offset", tuple->key, message_id, offset);

	if (message_id >= MAX_MESSAGE_COUNT)
	{
		return;
	}

	switch(offset)
	{
	case KEY_UUID:
		strncpy(s_messageData[message_id].url, tuple->value->cstring, MAX_URL_LENGTH);
		s_messageData[message_id].url[MAX_URL_LENGTH - 1] = 0;
		break;
	case KEY_SENDER:
		strncpy(s_messageData[message_id].sender, tuple->value->cstring, MAX_SENDER_LENGTH);
		s_messageData[message_id].sender[MAX_SENDER_LENGTH - 1] = 0;
		break;
	case KEY_SUBJECT:
		strncpy(s_messageData[message_id].subject, tuple->value->cstring, MAX_SUBJECT_LENGTH);
		s_messageData[message_id].subject[MAX_SUBJECT_LENGTH - 1] = 0;
		break;
	case KEY_UNREAD:
		s_messageData[message_id].unread = tuple->value->uint8 != 0;
		break;
	case KEY_NEW:
	case KEY_DELETED:
	default:
		break;
	}

	//LOGGER_SII("should we refresh the screen?", message_id, s_messageCount);
	if (message_id + 1 > s_messageCount)
	{
		s_messageCount = message_id + 1;
		// trigger refresh.
		s_hasLoaded = true;
		K9_APP_LOG(APP_LOG_LEVEL_DEBUG, "YES!!! %d %d", message_id, s_messageCount);
	}
	InboxRefresh();
}

void markDeleted(const char const * uuid)
{

}


void my_in_rcv_handler(DictionaryIterator *received, void *context) {
	Tuple *command = dict_find(received, 0);
	uint32_t max_size;
	DictionaryResult ret;

	if (command)
	{
		switch (command->value->uint8)
		{
		case eMTR_Update:
			// is the inbox screen open?  if so trigger a refresh
			{
				Tuple * tuple = 0;
				tuple = dict_read_first(received);
				while (tuple != 0)
				{
					addDataToMsgs(tuple);
					tuple = dict_read_next(received);
				}
			}
			break;
		case eMTR_ConfirmDelete:
			// is the inbox screen open?  if so trigger a refresh
			{
				Tuple * tuple =  dict_find(received,KEY_URL);
				int i = md_index(tuple->value->cstring);
				if (i >= 0)
				{
					s_messageData[i].deleted = true;
					InboxRefresh();
				}
			}
			break;

		case eMTR_Body:

			//LOGGER_S("Got body");
			max_size = BODY_BUFFER_SIZE;
			ret = dict_merge(&s_body,
				&max_size,
				received,
				false,
				md_BodyDictionaryUpdate, 0);
			// is the read screen open?  If so trigger a refresh
			if (ret == DICT_OK )
			{
				//LOGGER_S("body db merged");
				if(_isBodyComplete(&s_body))
				{
					LOGGER_S("Body done");
					ReadRefresh();
				}
			}
			else
			{
				LOGGER_S("failed body merge");
			}
			break;
		case eMTR_Config:
		{
			Tuple* bodySize = dict_find(received, KEY_BODY_TEXT_SIZE);
			Tuple* inboxSize = dict_find(received, KEY_INBOX_TEXT_SIZE);
			if (bodySize)
				s_bodyTextSize = bodySize->value->uint8;
			if (bodySize)
				s_inboxTextSize = inboxSize->value->uint8;
			break;
		}
		case eMTR_Reset:
		{
			//dict_write_begin(&s_inbox, s_inboxBuffer, INBOX_BUFFER_SIZE);
			//dict_write_end(&s_inbox);
			memset(s_messageKeys, 0, sizeof(s_messageKeys));
		    memset(s_messageData, 0, sizeof(s_messageData));

			dict_write_begin(&s_body, s_bodyBuffer, BODY_BUFFER_SIZE);
			dict_write_end(&s_body);
			s_messageCount = 0;
			s_hasLoaded = false;

			memset(s_messageKeys, 0, sizeof(s_messageKeys));
		}
		break;
		case eMTR_ErrorMsg:
		{
			Tuple* errorMsg = dict_find(received,KEY_MESSAGE);
			if (errorMsg->type == TUPLE_CSTRING)
			{
				PleaseWaitDebug(errorMsg->value->cstring);
			}
		}
			break;
		default:
			{
				LOGGER_SI("Got unknown ", command->value->uint8);
			}
			break;
		}
	}

	mq_pop();
}
/*
void itoa(char*buff, int num)
{
	int i = 0, temp_num = num, length = 0;

	if(num >= 0) {
		// count how many characters in the number
		while(temp_num) {
			temp_num /= 10;
			length++;
		}

		// assign the number to the buffer starting at the end of the
		// number and going to the begining since we are doing the
		// integer to character conversion on the last number in the
		// sequence
		for(i = 0; i < length; i++) {
		 	buff[(length-1)-i] = '0' + (num % 10);
			num /= 10;
		}
		buff[i] = '\0'; // can't forget the null byte to properly end our string
	}
}

*/

int md_getMessageCount()
{
	return s_messageCount;
}
/*
static const char const *_getString(int index, uint16_t offset)
{
	if (index >= s_messageCount || index < 0)
	{
		return 0;
	}
	uint16_t basekey = s_messageKeys[index];
	Tuple *entry = dict_find(&s_inbox, basekey + offset);
	if (entry)
	{
		if (entry->type == TUPLE_CSTRING)
		{
			return entry->value->cstring;
		}
	}
	return 0;
}

static bool _getBool(int index, uint16_t offset)
{
	if (index >= s_messageCount || index < 0)
	{
		return 0;
	}
	uint16_t basekey = s_messageKeys[index];
	Tuple *entry = dict_find(&s_inbox, basekey + offset);
	if (entry)
	{
		if (entry->type == TUPLE_UINT)
		{
			return entry->value->uint8 != 0;
		}
	}
	return 0;
}
*/

const char const * md_sender(int index)
{
	return s_messageData[index].sender;
	//return _getString(index, KEY_SENDER);
}

const char const *  md_date(int index)
{
	return 0;
}

const char const * md_subject(int index)
{
	return s_messageData[index].subject;
}

const char const * md_body()
{
	return s_bodyExtract;
}
const char const * md_uuid(int index)
{
	return s_messageData[index].url;
}

int md_index(const char const * uuid)
{
	int i;
	for (i =0; i < md_getMessageCount(); i++)
	{
		const char* const db_uuid = md_uuid(i);

		if (db_uuid)
		{
			if (strcmp(uuid, db_uuid) == 0)
			{
				return i;
			}
		}
	}
	return -1;
}
bool  md_unread(int index)
{
	return s_messageData[index].unread;
}

bool  md_isDeleted(int index)
{
	return s_messageData[index].deleted;
}

bool md_hasAttachment(int index)
{
	return false;
}
bool md_isNew(int index)
{
	return false;
}

DisplayTextSize md_inboxTextSize()
{
	return (DisplayTextSize)s_inboxTextSize;
}
DisplayTextSize md_bodyTextSize()
{
	return (DisplayTextSize)s_bodyTextSize;
}

