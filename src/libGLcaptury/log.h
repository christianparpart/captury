/////////////////////////////////////////////////////////////////////////////
//
//  Captury - http://rm-rf.in/captury
//  $Id$
//
//  Copyright (c) 2007 by Christian Parpart <trapni@gentoo.org>
//
//  This file as well as its whole library is licensed under
//  the terms of GPL. See the file COPYING.
//
/////////////////////////////////////////////////////////////////////////////
#ifndef sw_captury_libGLcaptury_log_h
#define sw_captury_libGLcaptury_log_h

#include <stdarg.h>

const int CAPTURY_LOG_ERROR = 0;
const int CAPTURY_LOG_WARNING = 1;
const int CAPTURY_LOG_NOTICE = 2;
const int CAPTURY_LOG_INFO = 3;
const int CAPTURY_LOG_DEBUG = 4;

const char *getTimeStamp();

void logError(const char *msg, ...);
void logWarning(const char *msg, ...);
void logNotice(const char *msg, ...);
void logInfo(const char *msg, ...);
void logDebug(const char *fmt, ...);

int verbosity();

void log(int level, const char *msg, va_list va);

#endif

// vim:ai:noet:ts=4:nowrap
