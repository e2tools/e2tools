SUBDIRS = lib .
ACLOCAL_AMFLAGS = -I m4

CLEANFILES =
EXTRA_DIST =
SUFFIXES =

bin_PROGRAMS = e2tools

e2tools_CFLAGS =
e2tools_CPPFLAGS =
e2tools_LDADD =

# e2tools_CFLAGS   += -save-temps=obj

e2tools_CPPFLAGS += $(EXT2FS_CFLAGS)
e2tools_LDADD    += $(EXT2FS_LIBS)

e2tools_CPPFLAGS += $(COM_ERR_CFLAGS)
e2tools_LDADD    += $(COM_ERR_LIBS)

e2tools_SOURCES = \
	copy.c copy.h \
	e2tools.c e2tools.h \
	elist.c elist.h \
	ln.c ln.h \
	ls.c ls.h \
	mkdir.c mkdir.h \
	mv.c mv.h \
	progress.c progress.h \
	read.c read.h \
	rm.c rm.h \
	tail.c tail.h \
	util.c util.h \
	write.c write.h
e2tools_SOURCES += compiler-definitions.h

EXTRA_DIST += README.md
EXTRA_DIST += TODO

e2tools_symlinks =
man_MANS =

dist_man_MANS = e2tools.7

if HAVE_EXT2FS_XATTRS
# EXTRA_DIST       += e2chcon.1in
# CLEANFILES       += e2chcon.1
# man_MANS         += e2chcon.1
# e2tools_symlinks += e2chcon
endif

dist_man_MANS    += e2cp.1
e2tools_symlinks += e2cp

dist_man_MANS    += e2ln.1
e2tools_symlinks += e2ln

EXTRA_DIST       += e2ls.1in
CLEANFILES       += e2ls.1
man_MANS         += e2ls.1
e2tools_symlinks += e2ls

dist_man_MANS    += e2mkdir.1
e2tools_symlinks += e2mkdir

dist_man_MANS    += e2mv.1
e2tools_symlinks += e2mv

dist_man_MANS    += e2rm.1
e2tools_symlinks += e2rm

dist_man_MANS    += e2tail.1
e2tools_symlinks += e2tail

SUFFIXES += .1in .1
.1in.1:
	$(SED) -e 's|[@]HAVE_EXT2FS_XATTRS_FALSE@|.\\" |g' < $< > $@

# This avoids relying on ln having a -f parameter.
install-exec-hook:
	cd $(DESTDIR)$(bindir) && \
	for symlink in $(e2tools_symlinks); do \
		rm -f "$${symlink}"; \
		$(LN_S) e2tools "$${symlink}" || exit 1; \
	done

uninstall-hook:
	-cd $(DESTDIR)$(bindir) && rm -f $(e2tools_symlinks)

clean-local:
	-rm -f *.i *.s

test:
	@echo "no tests yet"
