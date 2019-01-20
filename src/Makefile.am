uidir = $(datadir)/s2ssedit/ui
ui_DATA = s2ssedit.ui \
	phoenix.png \
	bomb.png \
	ring.png \
	ring.svg \
	bomb.svg \
	bomb-22.png \
	ring-22.png \
	s2ssanim.png \
	s2ssedit.png \
	ring-diamond.png \
	ring-line.png \
	ring-lozenge.png \
	ring-star.png \
	ring-triangle.png \
	ring-zig.png \
	bomb-diamond.png \
	bomb-line.png \
	bomb-lozenge.png \
	bomb-star.png \
	bomb-triangle.png \
	bomb-zig.png \
	bomb-loop.png \
	ring-loop.png \
	grid.png

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	$(S2SSEDIT_CFLAGS)

bin_PROGRAMS = s2ssedit

s2ssedit_SOURCES = \
	main.cc   \
	sseditor.cc   \
	sssegmentobjs.cc   \
	sslevelobjs.cc   \
	ssobjfile.cc

s2ssedit_CXXFLAGS = \
	-I$(srcdir)/../include \
	-I$(srcdir)/../mdcomp/include \
	$(EXTRA_FLAGS)

s2ssedit_LDFLAGS = -lkosinski -lnemesis

if BUILD_ICON
s2ssedit_DEPENDENCIES = $(builddir)/$(ICON_FILE)
s2ssedit_LDADD = $(S2SSEDIT_LIBS) $(builddir)/$(ICON_FILE)
else !BUILD_ICON
s2ssedit_DEPENDENCIES = $(ICON_FILE)
s2ssedit_LDADD = $(S2SSEDIT_LIBS)
endif
s2ssedit_LDADD += -L$(srcdir)/../mdcomp/src/lib/.libs

EXTRA_DIST = $(ui_DATA)

AM_CXXFLAGS =

$(builddir)/s2sseditico.o: $(srcdir)/ssedit.rc $(srcdir)/s2ssedit.ico
	windres --include-dir $(srcdir) $(srcdir)/ssedit.rc $(builddir)/s2sseditico.o

# Remove ui directory on uninstall
uninstall-local:
	-rm -r $(uidir)
	-rm -r $(datadir)/s2ssedit

