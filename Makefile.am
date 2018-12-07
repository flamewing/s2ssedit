SUBDIRS = src

ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

s2sseditdocdir = ${prefix}/doc/s2ssedit

s2sseditdoc_DATA = README.md COPYING INSTALL

EXTRA_DIST = $(s2sseditdoc_DATA) $(INTLTOOL_FILES)

DISTCLEANFILES =

# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(s2sseditdocdir)

