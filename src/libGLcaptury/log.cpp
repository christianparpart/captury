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
#include "log.h"

#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include <sys/types.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

int errorCount = 0;

const char *getTimeStamp() {
	time_t t = time(0);
	struct tm *tm = localtime(&t);

	static char timestamp[128];
	strftime(timestamp, sizeof(timestamp), "%F %T", tm);

	return timestamp;
}

void logError(const char *msg, ...) {
	va_list va;

	va_start(va, msg);
	log(CAPTURY_LOG_ERROR, msg, va);
	va_end(va);
}

void logWarning(const char *msg, ...) {
	va_list va;

	va_start(va, msg);
	log(CAPTURY_LOG_WARNING, msg, va);
	va_end(va);
}

void logNotice(const char *msg, ...) {
	va_list va;

	va_start(va, msg);
	log(CAPTURY_LOG_NOTICE, msg, va);
	va_end(va);
}

void logInfo(const char *msg, ...) {
	va_list va;

	va_start(va, msg);
	log(CAPTURY_LOG_INFO, msg, va);
	va_end(va);
}

void logDebug(const char *fmt, ...) {
#if !defined(NDEBUG)
	va_list va;

	fprintf(stderr, "%s CAPTURY(DD) ", getTimeStamp());
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	va_end(va);
	fprintf(stderr, "\n");
	fflush(stderr);
#endif
}

int verbosity() {
	static char *verbosity = NULL;
	static int level = 0;
	if (!verbosity) {
		verbosity = getenv("CAPTURY_VERBOSE");
		if (verbosity && *verbosity)
			level = atoi(verbosity);

			if (level > CAPTURY_LOG_DEBUG)
				level = CAPTURY_LOG_DEBUG;
			else if (level < CAPTURY_LOG_ERROR)
				level = CAPTURY_LOG_ERROR;
	}

	return level;
}

void log(int level, const char *msg, va_list va) {
	static int vlevel = verbosity();
	static const char *levelid[] = { "EE", "WW", "NN", "II", "DD" };
	if (level <= vlevel) {
		if (level == CAPTURY_LOG_ERROR)
			++errorCount;

		fprintf(stderr, "%s CAPTURY(%s) ", getTimeStamp(), levelid[level]);
		vfprintf(stderr, msg, va);
		fprintf(stderr, "\n");

		fflush(stderr);
	}
}

/*void log(int level, const char *msg, ...) {
	va_list va;

	va_start(va, msg);
	log(level, msg, va);
	va_end(va);
}*/

// vim:ai:noet:ts=4:nowrap
