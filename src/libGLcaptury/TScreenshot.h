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
#ifndef sw_captury_libGLcaptury_TScreenshot_h
#define sw_captury_libGLcaptury_TScreenshot_h

#include <png.h>

struct TScreenshot {
private:
	int width;
	int height;
	unsigned char *buffer;

public:
	static void capture();

private:
	TScreenshot(int AWidth, int AHeight);
	~TScreenshot();

	static void onWritePNG(png_structp APng, png_bytep ABuffer, png_size_t ASize);
	static void onFlushPNG(png_structp APng);
	static void *encodeScreenshot(void *arg);

	void encode();
};

#endif

// vim:ai:noet:ts=4:nowrap
