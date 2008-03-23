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
//#include "libX11.h"
#include "log.h"

#include <X11/Xlib.h>
#include <X11/keysym.h>

#define GLX_GLXEXT_PROTOTYPES // required for glXGetProcAddressARB when using xorg-x11 GL headers
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>

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

bool handleXEvent(Display *dpy, XEvent *event) {
	if (hookErrors || event->type != KeyPress)
		return false;

//	logDebug("keyPress event received");

	int keycode = event->xkey.keycode;
	if (keycode == XKeysymToKeycode(dpy, hotkeys.toggleCaptureMovie)) {
		captureMovie = !captureMovie;

//		logDebug("toggling movie capture (%s)", captureMovie ? "on" : "off");

		if (!captureMovie)
			stopMovieCapture();

		return true;
	} else if (keycode == XKeysymToKeycode(dpy, hotkeys.takeScreenshot)) {
//		logDebug("Initiated: `take screenshot`-action");
		takeScreenshot = true;

		return true;
	}

	return false;
}

int XPending(Display *dpy) {
	int rv = hooked.XPending(dpy);

	if (rv > 0) {
		XEvent ev;
		hooked.XPeekEvent(dpy, &ev);
		if (handleXEvent(dpy, &ev)) {
			hooked.XNextEvent(dpy, &ev);
			--rv;
		}
	}

	return rv;
}

int XNextEvent(Display *dpy, XEvent *event) {
	int rv;

	do rv = hooked.XNextEvent(dpy, event);
	while (handleXEvent(dpy, event));

	return rv;
}

int XPeekEvent(Display *dpy, XEvent *event) {
	int rv;

	rv = hooked.XPeekEvent(dpy, event);

	while (handleXEvent(dpy, event)) {
		hooked.XNextEvent(dpy, event);		// skip processed event
		rv = hooked.XPeekEvent(dpy, event);	// and peek next
	}

	return rv;
}

int XWindowEvent(Display *dpy, Window w, long mask, XEvent *event) {
	int rv;

	do rv = hooked.XWindowEvent(dpy, w, mask, event);
	while (handleXEvent(dpy, event));

	return rv;
}

Bool XCheckWindowEvent(Display *dpy, Window w, long mask, XEvent *event) {
	Bool rv;
	
	do rv = hooked.XCheckWindowEvent(dpy, w, mask, event);
	while (rv && handleXEvent(dpy, event));

	return rv;
}

int XMaskEvent(Display *dpy, long mask, XEvent *event) {
	int rv = hooked.XMaskEvent(dpy, mask, event);

	handleXEvent(dpy, event);

	return rv;
}

Bool XCheckMaskEvent(Display *dpy, long mask, XEvent *event) {
	Bool rv = hooked.XCheckMaskEvent(dpy, mask, event);

	if (rv)
		handleXEvent(dpy, event);

	return rv;
}

Bool XCheckTypedEvent(Display *dpy, int type, XEvent *event) {
	Bool rv = hooked.XCheckTypedEvent(dpy, type, event);

	if (rv)
		handleXEvent(dpy, event);

	return rv;
}

Bool XCheckTypedWindowEvent(Display *dpy, Window w, int type, XEvent *event) {
	Bool rv = hooked.XCheckTypedWindowEvent(dpy, w, type, event);

	if (rv)
		handleXEvent(dpy, event);

	return rv;
}

int XIfEvent(Display *dpy, XEvent *event, Bool(*predicate)(Display *, XEvent *, XPointer), XPointer arg) {
	logDebug("XIfEvent()");
	int rv = hooked.XIfEvent(dpy, event, predicate, arg);

	handleXEvent(dpy, event);

	return rv;
}

Bool XCheckIfEvent(Display *dpy, XEvent *event, Bool(*predicate)(Display *, XEvent *, XPointer), XPointer arg) {
//	logDebug("XCheckIfEvent()"); // wine (note: not cedega) is using this alot - at least on WoW/Launcher.exe
	Bool rv = hooked.XCheckIfEvent(dpy, event, predicate, arg);

	if (rv)
		handleXEvent(dpy, event);

	return rv;
}

int XPeekIfEvent(Display * dpy, XEvent * event, Bool(*predicate) (Display *, XEvent *, XPointer), XPointer arg) {
	logDebug("XPeekIfEvent()");
	int rv = hooked.XPeekIfEvent(dpy, event, predicate, arg);

	handleXEvent(dpy, event);

	return rv;
}

// vim:ai:noet:ts=4:nowrap
