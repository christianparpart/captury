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
#include "libGLcaptury.h"
#include "TScreenshot.h"
#include "log.h"
#include "freetype.h"

#include <captury/captury.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>

#define GLX_GLXEXT_PROTOTYPES // required for glXGetProcAddressARB when using xorg-x11 GL headers
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include <GL/glut.h> // for overlay only

#include <dlfcn.h>

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

// {{{ Overlay
//#define __TEST_CAPTURY_OVERLAY__ (1)

freetype::font_data font;

void overlayFPS() {
	static int i = 0;
	if (!i) {
		font.init("/home/trapni/test.ttf", 16);
		++i;
	}

	glColor3f(1, 0, 0);
	freetype::print(font, 0, 0, "Hello, World (%d)", ++i);
}

void drawOverlay() {
	glPushAttrib(GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_TRANSFORM_BIT);
	{
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_FOG);
		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		{
			glLoadIdentity();

			int viewPort[4];
			glGetIntegerv(GL_VIEWPORT, viewPort);

			glOrtho(0, viewPort[2], 0, viewPort[3], -1, 1);

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();
			{
				overlayFPS();
			}
			glPopMatrix();
			glMatrixMode(GL_PROJECTION);
		}
		glPopMatrix();
	}
	glPopAttrib();
}
// }}}

class TReadBufferOverride {
private:
	GLint FBackup;

public:
	inline TReadBufferOverride() :
		FBackup(GL_NONE) {}

	inline void override(GLenum AValue) {
		if (FBackup != GL_NONE)
			glGetIntegerv(GL_READ_BUFFER, &FBackup);

		glReadBuffer(AValue);
	}

	inline void restore() {
		if (FBackup != GL_NONE)
			glReadBuffer(FBackup);
	}
};

void glXSwapBuffers(Display *dpy, GLXDrawable drawable) {
	TReadBufferOverride rbo;

	if (captureMovie) {
		if (!cd) { // wanna capture, but not yet actually started (= first frame)
			if (FCurrentWidth == 0)
				updateGeometry(dpy, drawable);

			if (!startMovieCapture(dpy, drawable)) {
				captureMovie = false; // initialization failed

				hooked.glXSwapBuffers(dpy, drawable);
				return;
			}
		}

		// TODO draw water mark/logo, if enabled

		// make sure we're capturing from back buffer
		rbo.override(GL_BACK);

		CapturyProcessFrame(cd);

//		if (takeScreenshot) {
//			takeScreenshot = false;
//			// TODO reuse the already captured frame used for the movie, so we can make a screenshot of it.
//		}

		// TODO draw an overlay current capturing state (fps, MiB/s)
	}

	if (takeScreenshot) {
		if (FCurrentWidth == 0)
			updateGeometry(dpy, drawable);

		// make sure we're capturing from back buffer
		rbo.override(GL_BACK);

		TScreenshot::capture();

		takeScreenshot = false;
	}

#if defined(__TEST_CAPTURY_OVERLAY__)
	drawOverlay();
#endif

	rbo.restore();

	hooked.glXSwapBuffers(dpy, drawable);
}

__GLXextFuncPtr glXGetProcAddress(const GLubyte *procName) {
	static struct {
		const char *name;
		__GLXextFuncPtr func;
	} fnmap[] = {
		{ "glXGetProcAddressARB", (__GLXextFuncPtr)glXGetProcAddressARB },
		{ "glXSwapBuffers", (__GLXextFuncPtr)glXSwapBuffers },
		{ 0, 0 }
	};

//	logDebug("glXGetProcAddress: '%s'", procName);

	for (int i = 0; fnmap[i].name; ++i)
		if (!strcmp((char *)procName, fnmap[i].name))
			return fnmap[i].func;

	return (__GLXextFuncPtr) hooked.glXGetProcAddressARB(procName);
}

void (*glXGetProcAddressARB(const GLubyte *procName))() {
	return glXGetProcAddress(procName);
}

// vim:ai:noet:ts=4:nowrap
