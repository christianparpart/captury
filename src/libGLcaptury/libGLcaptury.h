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
#ifndef sw_captury_libGLcaptury_h
#define sw_captury_libGLcaptury_h

#include <captury/captury.h>

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

#include <sys/types.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

const char *programName();

// --------------------------------------------------------------------------
// hooks

#define HOOK(baseName) \
	this->baseName = (baseName##_fn) dlsym(RTLD_NEXT, #baseName); \
	if (this->baseName == 0) \
		hookError(#baseName);

void hookError(const char *baseName);

typedef int (*XPending_fn)(Display *dpy);
typedef int (*XNextEvent_fn)(Display *dpy, XEvent *event);
typedef int (*XPeekEvent_fn)(Display *dpy, XEvent *event);
typedef int (*XWindowEvent_fn)(Display *dpy, Window w, long mask, XEvent *event);
typedef Bool (*XCheckWindowEvent_fn)(Display *dpy, Window w, long mask, XEvent *event);
typedef int (*XMaskEvent_fn)(Display *dpy, long mask, XEvent *event);
typedef Bool (*XCheckMaskEvent_fn)(Display *dpy, long mask, XEvent *event);
typedef Bool (*XCheckTypedEvent_fn)(Display *dpy, int type, XEvent *event);
typedef Bool (*XCheckTypedWindowEvent_fn)(Display *dpy, Window w, int type, XEvent *event);

typedef int (*XIfEvent_fn)(Display *dpy, XEvent *event, Bool (*predicate)(Display *, XEvent *, XPointer), XPointer arg);
typedef Bool (*XCheckIfEvent_fn)(Display *dpy, XEvent *event, Bool (*predicate)(Display *, XEvent *, XPointer), XPointer arg);
typedef int (*XPeekIfEvent_fn)(Display *dpy, XEvent *event, Bool (*predicate)(Display *, XEvent *, XPointer), XPointer arg);

typedef void *(*glXGetProcAddressARB_fn)(const GLubyte *procName);
typedef void (*glXSwapBuffers_fn)(Display *dpy, GLXDrawable drawable);

struct TFunctionTable {
	glXGetProcAddressARB_fn glXGetProcAddressARB;
	glXSwapBuffers_fn glXSwapBuffers;

	XPending_fn XPending;
	XNextEvent_fn XNextEvent;
	XPeekEvent_fn XPeekEvent;
	XWindowEvent_fn XWindowEvent;
	XCheckWindowEvent_fn XCheckWindowEvent;
	XMaskEvent_fn XMaskEvent;
	XCheckMaskEvent_fn XCheckMaskEvent;

	XCheckTypedEvent_fn XCheckTypedEvent;
	XCheckTypedWindowEvent_fn XCheckTypedWindowEvent;

	XIfEvent_fn XIfEvent;
	XCheckIfEvent_fn XCheckIfEvent;
	XPeekIfEvent_fn XPeekIfEvent;

	void load();
};

// --------------------------------------------------------------------------
struct THotkeys {
	KeySym toggleCaptureMovie;
	KeySym takeScreenshot;
	//...
};

// ------------------------------------------------------------------------------------------------
void loadConfig();
const char *mediaFileNameBase();
const char *movieFileName();
const char *screenshotFileName();

// ------------------------------------------------------------------------------------------------
extern TFunctionTable hooked;
extern captury_t *cd;
extern bool takeScreenshot;
extern unsigned FCurrentWidth;
extern unsigned FCurrentHeight;
extern THotkeys hotkeys;
extern int hookErrors;

extern bool captureMovie;

void updateGeometry(Display *dpy, Drawable drawable);

bool startMovieCapture(Display *dpy, Window window);
void stopMovieCapture();

bool handleXEvent(Display *dpy, XEvent *event);

#endif

// vim:ai:noet:ts=4:nowrap
