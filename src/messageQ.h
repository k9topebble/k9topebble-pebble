/*
 * messageQ.h
 *
 *  Created on: 13 Jul 2013
 *      Author: andrew
 */

#ifndef MESSAGEQ_H_
#define MESSAGEQ_H_

typedef void (* MQ_CALLBACK)(DictionaryIterator*, AppMessageResult reason,  void*);

void mq_post(DictionaryIterator*);
void mq_post_cb(DictionaryIterator*, MQ_CALLBACK, void *);
void mq_create();
void mq_pop();
#endif /* MESSAGEQ_H_ */
