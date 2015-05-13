/*
 * ScreenCapture.c
 *
 *  Created on: 23 Jul 2013
 *      Author: andrew
 */
#include "pebble_app.h"
#include "pebble_os.h"
#include "pebble_app_info.h"
#include "stdbool.h"
#include "defines.h"
#include "messageQ.h"

#define PBL_CAPTURE_COOKIE 0x70626c63 // 'pblc'

// The *hack* to get to the framebuffer
struct GContext {
	void **ptr;
};

#define BODY_LEN 84
#define IMAGE_SIZE (168*18)

#ifdef SCREEN_CAPTURE_ENABLED
static unsigned char http_capture_frameBuffer[18*168];

void pbl_capture_send() {
	struct GContext *gctx = app_get_current_graphics_context();
	unsigned char *ptr = (unsigned char *)(*gctx->ptr);
	int pbl_capture_sentLen = 0;

	DictionaryResult ret;
	DictionaryIterator imgChunk;
	uint8_t buff[MAX_OUT_MESSAGE_SIZE];

	int len = 0;

	for (int y=0; y<168; y++) {
		for (int x=0; x<18; x++) {
			http_capture_frameBuffer[len++] = *ptr++;
		}
		ptr++; ptr++;
	}

	app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);

	while (pbl_capture_sentLen < IMAGE_SIZE)
	{
		DictionaryResult res = dict_write_begin(&imgChunk, buff, MAX_OUT_MESSAGE_SIZE);
		if (res != DICT_OK)
		{
			return;
		}
		uint8_t msg =  mMT_SendImage;
		ret = dict_write_int(&imgChunk, KEY_COMMAND, &msg, 1, false);
		if (DICT_OK != ret)
		{
			return;
		}

		uint16_t pos = pbl_capture_sentLen;
		ret = dict_write_int(&imgChunk, KEY_IMG_SIZE, &pos, 2, false);
		if (DICT_OK != ret)
		{
			return;
		}

		ret = dict_write_data(&imgChunk, KEY_IMG_START, http_capture_frameBuffer + pbl_capture_sentLen, BODY_LEN);
		if (DICT_OK != ret)
		{
			return;
		}
		pbl_capture_sentLen += BODY_LEN;

		dict_write_end(&imgChunk);

		mq_post(&imgChunk);
	}
}

#else

void pbl_capture_send()
{

}

#endif
