/*
 * AreYouSure.h
 *
 *  Created on: 25 Jul 2013
 *      Author: andrew
 */
#include "stdbool.h"

#ifndef AREYOUSURE_H_
#define AREYOUSURE_H_

typedef void (* AYS_CALLBACK)(void*);


void AreYouSureCreate();
void AreYouSureShow(const char const *, AYS_CALLBACK);
void AreYouSureHide();
bool AreYouSureConfirm();

#endif /* AREYOUSURE_H_ */
