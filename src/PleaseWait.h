/*
 * PleaseWait.h
 *
 *  Created on: 6 Jul 2013
 *      Author: andrew
 */
#include "stdbool.h"

#ifndef PLEASEWAIT_H_
#define PLEASEWAIT_H_

void PleaseWaitCreate();
void PleaseWaitShow(const char const *);
void PleaseWaitHide();
bool PleaseWaitActive();
void PleaseWaitDebug(const char const *);

#endif /* PLEASEWAIT_H_ */
