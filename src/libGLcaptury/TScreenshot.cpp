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
#include "TScreenshot.h"
#include "libGLcaptury.h"
#include "log.h"

#include <pthread.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

#define GLX_GLXEXT_PROTOTYPES // required for glXGetProcAddressARB when using xorg-x11 GL headers
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>

#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include <png.h>

#include <sys/types.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

void TScreenshot::capture() {
//	printf("TScreenshot::capture()\n");
	TScreenshot *shot = new TScreenshot(FCurrentWidth, FCurrentHeight);

	glReadPixels(0, 0, shot->width, shot->height, GL_RGB, GL_UNSIGNED_BYTE, shot->buffer);

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); // do not wait for me.
	pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);

	pthread_t thid;
	pthread_create(&thid, &attr, &encodeScreenshot, shot);
}

TScreenshot::TScreenshot(int AWidth, int AHeight) {
	width = FCurrentWidth;
	height = FCurrentHeight;
	buffer = new unsigned char[width * height * 3];
}

TScreenshot::~TScreenshot() {
	delete[] buffer;
}

void TScreenshot::onWritePNG(png_structp APng, png_bytep ABuffer, png_size_t ASize) {
	int fd = *static_cast<int *>(APng->io_ptr);
	write(fd, ABuffer, ASize);
}

void TScreenshot::onFlushPNG(png_structp APng) {
}

void *TScreenshot::encodeScreenshot(void *arg) {
	TScreenshot *self = (TScreenshot *)arg;
	self->encode();
	return 0;
}

void TScreenshot::encode() {
	const char *fileName = screenshotFileName();
	logInfo("Saving screenshot: %s", fileName);

	int fd = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (!fd) {
		logError("Could not open file for screenshot: %s", strerror(errno));
		return;
	}

	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, (void *)0, 0, 0);

	png_infop info = png_create_info_struct(png);
	if (!info) {
		png_destroy_write_struct(&png, 0);
		return;
	}

	// setjmp is required for default error handling (only)
	if (setjmp(png_jmpbuf(png))) {
		png_destroy_write_struct(&png, &info);
		return;
	}

	png_set_write_fn(png, &fd, &TScreenshot::onWritePNG, &TScreenshot::onFlushPNG);
	png_set_compression_level(png, 4);

	info->width = width;
	info->height = height;
	info->bit_depth = 8;
	info->color_type = PNG_COLOR_TYPE_RGB;

	png_write_info(png, info);

	const int lineStride = width * 3;
	for (int y = info->height - 1; y >= 0; --y)
		png_write_row(png, (png_bytep)buffer + y * lineStride);

	png_write_end(png, info);
	png_destroy_write_struct(&png, &info);

	close(fd);
}

// vim:ai:noet:ts=4:nowrap
