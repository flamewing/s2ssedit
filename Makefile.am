## Process this file with automake to produce Makefile.in
## Created by Anjuta

ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

SUBDIRS = src

s2sseditdocdir = ${prefix}/doc/s2ssedit

s2sseditdoc_DATA = \
	README \
	COPYING \
	INSTALL

EXTRA_DIST = $(s2sseditdoc_DATA) \
	$(INTLTOOL_FILES)

DISTCLEANFILES =


# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(s2sseditdocdir)

