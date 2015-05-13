/*
 * logger.c
 *
 *  Created on: 12 Jul 2013
 *      Author: andrew
 */

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include "pebble_os.h"
#include "defines.h"
#include <stdarg.h>
#include "PleaseWait.h"
#include "messageQ.h"


static const char const * _distmsg(DictionaryResult ret)
{
	switch (ret)
	{
	case DICT_OK: return "DICT_OK";
	case DICT_NOT_ENOUGH_STORAGE: return "DICT_NOT_ENOUGH_STORAGE";
	case DICT_INVALID_ARGS: return "DICT_INVALID_ARGS";
	case DICT_INTERNAL_INCONSISTENCY: return "DICT_INTERNAL_INCONSISTENCY";
	default: return "bad value";
	}
}

static const char const * _appResult(AppMessageResult result)
{
	switch (result)
	{

	case APP_MSG_OK:
		return "APP_MSG_OK";
	case APP_MSG_SEND_TIMEOUT:
		return "APP_MSG_SEND_TIMEOUT";
	case APP_MSG_SEND_REJECTED:
		return "APP_MSG_SEND_REJECTED";
	case APP_MSG_NOT_CONNECTED:
		return "APP_MSG_NOT_CONNECTED";
	case APP_MSG_APP_NOT_RUNNING:
		return "APP_MSG_APP_NOT_RUNNING";
	case APP_MSG_INVALID_ARGS:
		return "APP_MSG_INVALID_ARGS";
	case APP_MSG_BUSY:
		return "APP_MSG_BUSY";
	case APP_MSG_BUFFER_OVERFLOW :
		return "APP_MSG_BUFFER_OVERFLOW";
	case APP_MSG_ALREADY_RELEASED:
		return "APP_MSG_ALREADY_RELEASED";
	case APP_MSG_CALLBACK_ALREADY_REGISTERED:
		return "APP_MSG_CALLBACK_ALREADY_REGISTERED";
	case APP_MSG_CALLBACK_NOT_REGISTERED:
		return "APP_MSG_CALLBACK_NOT_REGISTERED";

	default:
		return "unknown message";
	}
}

#define MAX_OUT_MESSAGE_SIZE 128

void logger_msg(int count, ...)
{
	uint8_t buffer[MAX_OUT_MESSAGE_SIZE];

	DictionaryIterator command;

	DictionaryResult res = dict_write_begin(&command, buffer, MAX_OUT_MESSAGE_SIZE);
	if (res != DICT_OK)
	{
		PleaseWaitDebug(_distmsg(res));
		return;
	}


	uint8_t msg =  eMT_RequestLog;
	res = dict_write_int(&command, 0, &msg, 1, false);
	if (DICT_OK != res)
	{
		PleaseWaitDebug(_distmsg(res));
		return;
	}
	{
		char *s;
		uint32_t uivalue;
		int32_t ivalue;
		va_list argp;
		bool error = false;
		// loop of type then value
		va_start(argp, count);
		for (int i = 0; i < count; i++)
		{
			TupleType ttype = va_arg(argp, int);
			switch(ttype)
			{
			case TUPLE_CSTRING:
				s = va_arg(argp, char *);
				res = dict_write_cstring(&command, 1 + i,s);
				if (DICT_OK != res)
				{
					PleaseWaitDebug("CSTRING failed");
					error = true;
				}
				break;
			case TUPLE_UINT:
				uivalue = va_arg(argp, uint32_t);
				res = dict_write_int(&command, 1 + i, &uivalue, 4, false);

				if (DICT_OK != res)
				{
					PleaseWaitDebug("UINT failed");
					error = true;
				}
				break;
			case TUPLE_INT:
				ivalue = va_arg(argp, int32_t);
				res = dict_write_int(&command, 1 + i, &ivalue, 4, true);

				if (DICT_OK != res)
				{
					PleaseWaitDebug("INT failed");
					error = true;
				}
				break;

			default:
				error = true;
				PleaseWaitDebug("Send failed bad type");
				break;
				// failed
			}
			if (error)
			{
				va_end(argp);
				//PleaseWaitDebug("send failed with error");
				return;
			}
		}

		va_end(argp);
	}
	//PleaseWaitDebug("Log post");
	dict_write_end(&command);
	mq_post(&command);
	return;

}

