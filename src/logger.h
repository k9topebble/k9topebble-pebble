/*
 * logger.h
 *
 *  Created on: 12 Jul 2013
 *      Author: andrew
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include "defines.h"

void logger_msg(int count, ...);

#ifdef DEBUG
#define LOGGER_S(s) //logger_msg(1, TUPLE_CSTRING, (s))
#define LOGGER_SI(s, i) logger_msg(2, TUPLE_CSTRING, (s), TUPLE_INT, (i))
#define LOGGER_SII(s, i, i2) logger_msg(3, TUPLE_CSTRING, (s), TUPLE_INT, (i), TUPLE_INT, (i2))
#define LOGGER_SIII(s, i, i2, i3) logger_msg(4, TUPLE_CSTRING, (s), TUPLE_INT, (i), TUPLE_INT, (i2), TUPLE_INT, (i3))
#define LOGGER_SS(s, s2) //logger_msg(2, TUPLE_CSTRING, (s), TUPLE_CSTRING, (s2))
#else
#define LOGGER_S(s)
#define LOGGER_SI(s, i)
#define LOGGER_SII(s, i, i2)
#define LOGGER_SS(s, s2)
#endif
#endif /* LOGGER_H_ */
