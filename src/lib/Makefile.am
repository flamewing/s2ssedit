lib_LTLIBRARIES =

noinst_LIBRARIES = libstaticanalyzer.a

libstaticanalyzer_a_SOURCES = \
	abstractaction.cc \
	ignore_unused_variable_warning.cc \
	object.cc \
	sseditor.cc \
	sslevelobjs.cc \
	ssobjfile.cc \
	sssegmentobjs.cc

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(abs_top_srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-I$(abs_top_srcdir)/include \
	-I$(abs_top_srcdir)/mdcomp/include \
	$(S2SSEDIT_CFLAGS)

AM_LDFLAGS = -no-undefined
