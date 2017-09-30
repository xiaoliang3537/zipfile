/*
 * Debug.h
 *
 *  Created on:
 *      Author: xxx
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#include <iostream>
#include <memory>
#include <string>
#include <stdarg.h>
#ifdef ANDROID
#include <android/log.h>
#endif

#include "StringUtils.h"

#define JNIDEBUG 1

static int my_printf(const char *format, ...) {

#if JNIDEBUG

	va_list ap;
	va_start(ap, format);
#ifdef ANDROID
    __android_log_vprint(ANDROID_LOG_WARN, "ijm-debug", format, ap);
#else
	printf(format,ap);
#endif
	va_end(ap);

#endif
	return 0;
}
static int my_printf2(char *tag,const char *format, ...) {
#if JNIDEBUG
	va_list ap;
	va_start(ap, format);
#ifdef ANDROID
	__android_log_vprint(ANDROID_LOG_WARN, tag, format, ap);
#else
	printf(format,ap);
#endif
	va_end(ap);
#endif
	return 0;
}

 

#if 1
#if JNIDEBUG
#define ILOG my_printf2
#else
#define ILOG(TAG, format, args...) do {}while(0);
#endif
#else
#define ILOG(TAG, format, args...) do { \
		const char *pFileName=strrchr(__FILE__,'/')+1; \
		char msybuf[64]; \
		sprintf(msybuf,"%s - %s (%d) :  ", pFileName,__FUNCTION__, __LINE__);\
		my_printf2(TAG,msybuf);\
		my_printf2(TAG,format,args); \
}while(0);
#endif

#endif /* DEBUG_H_ */
