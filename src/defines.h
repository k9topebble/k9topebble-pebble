/*
 * logger.h
 *
 *  Created on: 12 Jul 2013
 *      Author: andrew
 */

#ifndef DEFINES_H_
#define DEFINES_H_

//#define DEBUG


#define MAJOR_VERSION 1
#define MINOR_VERSION 10
#define PROTOCOL_VERSION 6

#define VERSION_STRING "K9ToPebble v1.10"

#define MAX_OUT_MESSAGE_SIZE 128
//#define INBOX_BUFFER_SIZE (4 * 1024)
#define BODY_BUFFER_SIZE 2048
#define MAX_MESSAGE_COUNT 25  // this is the max nummber of messages we can cope with
#define MAX_BODY_SIZE 1024

#define MAX_URL_LENGTH 100
#define MAX_SENDER_LENGTH 50
#define MAX_SUBJECT_LENGTH 100

//#define SCREEN_CAPTURE_ENABLED

#ifdef DEBUG
	#define MAX_OUT_MESSAGE 50
#else
	#ifdef SCREEN_CAPTURE_ENABLED
		#define MAX_OUT_MESSAGE 40
	#else
		#define MAX_OUT_MESSAGE 4
	#endif
#endif

typedef enum _MessageTypesCommands
{
	eMT_RequestStart,
	eMT_RequestBody,
	eMT_RequestStop,
	eMT_RequestLog,
	eMT_SendImage,
	eMT_RequestMissing,
	eMT_ReplyPong,
	eMT_RequestDelete,
	eMT_ConfirmTag,
} MessageTypesCommands;

typedef enum _MessageTypesResponse
{
	eMTR_Reset = 50,
	eMTR_Update,
	eMTR_Body,
	eMTR_ErrorMsg,
	eMTR_RequestPing,
	eMTR_ConfirmDelete,
	eMTR_Config,
} MessageTypesResponse;

typedef enum _DisplayTextSize
{
	eSize_Regular,
	eSize_Small,
	eSize_Large,
} DisplayTextSize;

// defined key fields
#define KEY_COMMAND  0
#define KEY_URL      1
#define KEY_PROTOCOL_VERSION 2
#define KEY_MESSAGE  3
#define KEY_IMG_SIZE 4
#define KEY_EXPECTED 5
#define KEY_LAST     6
#define KEY_PONG_TAG 7
#define KEY_BODY_TEXT_SIZE 8
#define KEY_INBOX_TEXT_SIZE 9
#define KEY_IMG_START 100

//offsets
#define KEY_UUID 0
#define KEY_SENDER 1
#define KEY_SUBJECT 2
#define KEY_UNREAD 3
#define KEY_NEW 4
#define KEY_DELETED 5


#define KEY_START 100
#define KEY_INCREMENT 100
#endif /* DEFINES_H_ */

