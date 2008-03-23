AM_CXXFLAGS = -ansi -pedantic -Wall @CAPTURY_CFLAGS@ -DVERSION="\"@CAPTURY_VERSION@\"" -I/usr/include/freetype2
AM_CFLAGS = -std=c99 -Wall -D_GNU_SOURCE=1

# FIXME libGLnative.so and libX11native.so are always rebuilt - why?

NATIVE_LIBGL = /usr/lib/libGL.so
NATIVE_LIBX11 = /usr/lib/libX11.so

# ---------------------------------------------------------------------------
if BUILTIN_STRIP_SONAME
noinst_PROGRAMS = strip-soname

strip_soname_SOURCES = strip-soname.c
strip_soname_LDFLAGS = -lelf

STRIP_SONAME = $(top_builddir)/src/libGLcaptury/strip-soname
DEPEND_STRIP_SONAME = strip-soname
else
STRIP_SONAME = strip-soname
DEPEND_STRIP_SONAME = $(STRIP_SONAME)
endif

# ---------------------------------------------------------------------------
libGL.so: libGL.so.1
	ln -sf $< $@

libGL.so.1: libGLnative.so
	ln -sf libGLcaptury.so $@

libGLnative.so: $(NATIVE_LIBGL) $(DEPEND_STRIP_SONAME)
	cp -p $< $@
	$(STRIP_SONAME) $@

# ---------------------------------------------------------------------------
libX11.so: libX11.so.6
	ln -sf $< $@

libX11.so.6: libGLnative.so
	ln -sf libGLcaptury.so $@

libX11native.so: $(NATIVE_LIBX11) $(DEPEND_STRIP_SONAME)
	cp -p $< $@
	$(STRIP_SONAME) $@

# ---------------------------------------------------------------------------

%.o: %.cpp
	$(CXX) -c -shared -fPIC $(AM_CXXFLAGS) $(CXXFLAGS) -o $@ $<

libGLcaptury.so: libGLcaptury.o log.o libGL.o libX11.o TScreenshot.o freetype.o libGLnative.so libX11native.so
	$(CXX) -shared -fPIC -o $@ $^ \
		$(AM_CXXFLAGS) $(CXXFLAGS) $(CAPTURY_LIBS) $(PNG_LIBS) \
		-L. -lGLnative -lX11native -lfreetype

# ---------------------------------------------------------------------------
all-local: libGLcaptury.so libGL.so libX11.so

clean-local:
	rm -f libGLcaptury.so
	rm -f libGLnative.so libGL.so.1 libGL.so
	rm -f libX11native.so libX11.so.6 libX11.so
	rm -f strip-soname

# ---------------------------------------------------------------------------
install-exec-local: install-libGLnative install-libX11native install-libGLcaptury

capturylibdir = $(libdir)/captury

install-libGLnative:
	mkdir -p $(DESTDIR)$(capturylibdir)
	ln -sf $(NATIVE_LIBGL) $(DESTDIR)$(capturylibdir)/libGLnative.so
	ln -sf libGL.so.1 $(DESTDIR)$(capturylibdir)/libGL.so
	ln -sf libGLcaptury.so $(DESTDIR)$(capturylibdir)/libGL.so.1

install-libX11native:
	mkdir -p $(DESTDIR)$(capturylibdir)
	ln -sf $(NATIVE_LIBX11) $(DESTDIR)$(capturylibdir)/libX11native.so
	ln -sf libX11.so.6 $(DESTDIR)$(capturylibdir)/libX11.so
	ln -sf libGLcaptury.so $(DESTDIR)$(capturylibdir)/libX11.so.6

install-libGLcaptury:
	mkdir -p $(DESTDIR)$(capturylibdir)
	$(INSTALL) -m 0755 libGLcaptury.so $(DESTDIR)$(capturylibdir)/

# ---------------------------------------------------------------------------
uninstall-local: uninstall-libGLnative uninstall-libX11native uninstall-libGLcaptury

uninstall-libGLnative:
	rm -f $(DESTDIR)$(capturylibdir)/libGL.so
	rm -f $(DESTDIR)$(capturylibdir)/libGL.so.1
	rm -f $(DESTDIR)$(capturylibdir)/libGLnative.so
	rmdir $(DESTDIR)$(capturylibdir) || true

uninstall-libX11native:
	rm -f $(DESTDIR)$(capturylibdir)/libX11.so
	rm -f $(DESTDIR)$(capturylibdir)/libX11.so.6
	rm -f $(DESTDIR)$(capturylibdir)/libX11native.so
	rmdir $(DESTDIR)$(capturylibdir) || true

uninstall-libGLcaptury:
	rm -f $(DESTDIR)$(capturylibdir)/libGLcaptury.so
	rmdir $(DESTDIR)$(capturylibdir) || true

# vim:ai:noet:ts=4:nowrap
