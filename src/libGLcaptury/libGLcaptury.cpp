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

#include <captury/captury.h>

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

#include <png.h>

#include <sys/types.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

#include <sys/statvfs.h>

// ------------------------------------------------------------------------------------------------
//
// ENVIRONMENT VARIABLES:
//
// 		CAPTURY_OUTPUT_DIR	      : base directory of where to store the captured movies (default: $HOME)
//		CAPTURY_FPS			      : the fps to capture with (default: 25)
//		CAPTURY_SCALE		      : image down scaling (default: 0)
//		CAPTURY_HOTKEY_MOVIE      : defines the hotkey for toggling movie capture (default: F12)
//		CAPTURY_HOTKEY_SCREENSHOT : defines hotkey for taking screenshot (default: F10)
//		CAPTURY_EXPECTED	      : enforces exit(1), if set to any value and a hook resolv error happend
//
// TODO
//
// * all predicated Xlib functions: (XIfEvent, XCheckIfEvent...)
//   we shall use our own predicate to hook our handleXEvent() inside the predicate
//   and pass any remaining event to `predicate` until it returns True.
// * print HUD with FPS and MiB/s state on capture

// {{{ programName()
bool isPathSeperator(char AChar) {
	return AChar == '/' || AChar == '\\';
}

const char *programName() {
	static char buf[1024] = { 0 };

	if (buf[0] == '\0') {
		int i = readlink("/proc/self/exe", buf, sizeof(buf));
		if (i != -1) {
			// extract program name from full path
			buf[i] = 0;
			for (--i; i >= 0 && !isPathSeperator(buf[i]); --i);
			if (isPathSeperator(buf[i]))
				strcpy(buf, buf + i + 1);

			if (strcmp(buf, "wine-preloader") == 0) {
				// in case of wine/cedega, handle the application name with slight more precision
				int fd = open("/proc/self/cmdline", O_RDONLY);
				assert(fd != 0);

				char cmdline[4096];
				int nread = read(fd, cmdline, sizeof(cmdline));
				assert(nread != -1);
				cmdline[nread] = 0;

				char *args[8];
				int nargs = 0;

				for (char *p = cmdline; *p && nargs < int(sizeof(args) / sizeof(args[0])); ++p)
					for (args[nargs++] = p; *p; ++p);

				for (i = 1; i < nargs; ++i) { // cedega should match here
					if (strcmp(args[i - 1], "--") == 0) {
						strncpy(buf, args[i], sizeof(buf));
						break;
					}
				}

				if (i == nargs) // otherwise, it's wine
					strncpy(buf, args[0], sizeof(buf));

				for (i = strlen(buf) - 1; i >= 0 && !isPathSeperator(buf[i]); --i);

				if (isPathSeperator(buf[i]))
					strcpy(buf, buf + i + 1);
			}
		} else {
			logWarning("Error resolving program name: %s", strerror(errno));
			strcpy(buf, "unknown");
		}
	}
	return buf;
}
// }}}

// {{{ hooks
int hookErrors = 0;

void hookError(const char *baseName) {
	++hookErrors;

	logError("Could not hook into function %s: %s", baseName, dlerror());

	if (getenv("CAPTURY_EXPECTED") != 0)
		exit(1);
}

void TFunctionTable::load() {
	HOOK(glXGetProcAddressARB);
	HOOK(glXSwapBuffers);

	HOOK(XPending);
	HOOK(XNextEvent);
	HOOK(XPeekEvent);
	HOOK(XWindowEvent);
	HOOK(XCheckWindowEvent);
	HOOK(XMaskEvent);
	HOOK(XCheckMaskEvent);
	HOOK(XCheckTypedEvent);
	HOOK(XCheckTypedWindowEvent);

	HOOK(XIfEvent);
	HOOK(XCheckIfEvent);
	HOOK(XPeekEvent);
}
// }}}

// ------------------------------------------------------------------------------------------------
// runtime state vars
TFunctionTable hooked;
captury_t *cd = 0;

bool captureMovie = false;
bool takeScreenshot = false;

// global config vars
THotkeys hotkeys;
char outputBaseDir[1024] = { 0 };
double fps = 25.0;
int scale = 0;
bool showCursor = false;
int FRequiredDiskMB = 1024;
int FRequiredDiskPercent = 10;

// ------------------------------------------------------------------------------------------------

// \param AFileName the filename to check available diskspace for.
// \param AMegaBytes the number of MB of diskspace available on the mounted medium \p AFileName belongs to.
// \param APercent the percentage of diskspace available on the underlying mount-point.
// \return 0 on success
// \return -1 on error
int diskspaceAvailable(const char *AFileName, int& AMegaBytes, int& APercent) {
	struct statvfs stfs;
	if (statvfs(AFileName, &stfs) == -1)
		return -1;

	AMegaBytes = stfs.f_bavail * (stfs.f_bsize / 1024) / 1024;
	APercent = stfs.f_bavail * 100.0 / stfs.f_blocks;

	return 0;
}

bool checkAvailableDiskspace(bool AComplain) {
	int mbAvail, percentAvail;

	if (diskspaceAvailable(outputBaseDir, mbAvail, percentAvail) == -1)
		logWarning("Error determining available diskspace on %s: %s\n", outputBaseDir, strerror(errno));
	else if (mbAvail >= 1024)
		logInfo("Diskspace available on %s: %.3fGB, %d%%", outputBaseDir, mbAvail / 1024.0, percentAvail);
	else
		logInfo("Diskspace available on %s: %dMB, %d%%", outputBaseDir, mbAvail, percentAvail);

	if (mbAvail < FRequiredDiskMB || percentAvail < FRequiredDiskPercent) {
		if (AComplain)
			logError("You do not meet the available disk space requirements of %.3fGB and %d%%.",
					FRequiredDiskMB / 1024.0, FRequiredDiskPercent);
		else
			logWarning("You do not meet the available disk space requirements of %.3fGB and %d%%.",
					FRequiredDiskMB / 1024.0, FRequiredDiskPercent);

		return false;
	}

	return true;
}

void loadConfig() {
	// - /etc/captury.conf
	// - $HOME/.config/captury.conf

	// determine hotkey for toggling movie capture
	char *value = getenv("CAPTURY_HOTKEY_MOVIE");
	if (!value || !*value)
		value = "F12";

	hotkeys.toggleCaptureMovie = XStringToKeysym(value);

	// screenshot hotkey...
	value = getenv("CAPTURY_HOTKEY_SCREENSHOT");
	if (!value || !*value)
		value = "F10";

	hotkeys.takeScreenshot = XStringToKeysym(value);

	// auto-start movie capturing
	value = getenv("CAPTURY_AUTO_CAPTURE");
	if (value && (!strcasecmp(value, "yes") || !strcmp(value, "1")))
		captureMovie = true;

	// explicit cursor drawing?
	value = getenv("CAPTURY_CURSOR");
	if (value && (!strcasecmp(value, "yes") || !strcmp(value, "1")))
		showCursor = true;

	// determine output base dir (must not contain trailing slash)
	if (getenv("CAPTURY_OUTPUT_DIR") != 0)
		strncpy(outputBaseDir, getenv("CAPTURY_OUTPUT_DIR"), sizeof(outputBaseDir));

	if (!outputBaseDir[0]) {
		char *defaultBaseDir = getenv("HOME");
		if (!defaultBaseDir)
			defaultBaseDir = ".";

		strncpy(outputBaseDir, defaultBaseDir, sizeof(outputBaseDir));
	}

	if (outputBaseDir[strlen(outputBaseDir) - 1] == '/')
		outputBaseDir[strlen(outputBaseDir) - 1] = 0;

	// determine fps to capture with
	if (getenv("CAPTURY_FPS") != 0) {
		fps = strtod(getenv("CAPTURY_FPS"), 0);
		if (!fps)
			fps = 25;
	}

	// determine video scaling value
	if (getenv("CAPTURY_SCALE") != 0) {
		scale = atoi(getenv("CAPTURY_SCALE"));
	}

	value = getenv("CAPTURY_REQUIRE_DISK_MB");
	if (value && *value) {
		FRequiredDiskMB = atoi(value);
	}

	value = getenv("CAPTURY_REQUIRE_DISK_PERCENT");
	if (value && *value) {
		FRequiredDiskPercent = atoi(value);
	}
}

const char *mediaFileNameBase() {
	static char result[1024];
	snprintf(result, sizeof(result), "%s/%s - %s", outputBaseDir, programName(), getTimeStamp());

	return result;
}

const char *movieFileName() {
	static char result[1024];
	snprintf(result, sizeof(result), "%s.cps", mediaFileNameBase());
	return result;
}

const char *screenshotFileName() {
	static char result[1024];
	snprintf(result, sizeof(result), "%s.png", mediaFileNameBase());
	return result;
}

// ------------------------------------------------------------------------------------------------
unsigned FCurrentWidth = 0;
unsigned FCurrentHeight = 0;

void updateGeometry(Display *dpy, Drawable drawable) {
	int stmp;
	unsigned utmp;

	Window root;
	XGetGeometry(dpy, drawable, &root, &stmp, &stmp, &FCurrentWidth, &FCurrentHeight, &utmp, &utmp);
}

bool startMovieCapture(Display *dpy, Window window) {
	if (!checkAvailableDiskspace(true))
		return false;

	captury_config_t config;
	bzero(&config, sizeof(config));

	config.x = 0;
	config.y = 0;
	config.width = FCurrentWidth;
	config.height = FCurrentHeight;
	config.fps = fps;
	config.scale = scale;
	config.deviceType = CAPTURY_DEVICE_GLX;
	config.deviceHandle = dpy;
	config.windowHandle = window;
	config.cursor = showCursor;

	cd = CapturyOpen(&config);
	if (!cd) {
		logError("could not open captury device");
		return false;
	}

	const char *fileName = movieFileName();
	if (CapturySetOutputFileName(cd, fileName) == -1) {
		logError("error setting output stream: %s", strerror(errno));
		CapturyClose(cd);
		cd = 0;
	}

	logNotice("initiated movie capture at %dx%d+%d+%d: %s", config.width, config.height, config.x, config.y, fileName);

	return true;
}

void stopMovieCapture() {
	if (!cd)
		return;

	logNotice("stopping movie capture");
	CapturyClose(cd);
	cd = 0;
}

// ------------------------------------------------------------------------------------------------
__attribute__((constructor))
void initialize() {
	logInfo("initializing (%s)", programName());

	hooked.load();
	loadConfig();

	checkAvailableDiskspace(false);
}

__attribute__((destructor))
void finalize() {
	if (cd) {
		stopMovieCapture();
	}
}

// vim:ai:noet:ts=4:nowrap
