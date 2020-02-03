SUBDIRS = lib .
ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = e2tools

e2tools_CFLAGS =
e2tools_CPPFLAGS =
e2tools_LDADD =

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

EXTRA_DIST =
EXTRA_DIST += README.md
EXTRA_DIST += TODO

dist_man_MANS = e2tools.7
dist_man_MANS += e2cp.1 e2ln.1 e2ls.1 e2mkdir.1 e2mv.1 e2rm.1 e2tail.1

install-exec-hook:
	$(LN_S) -f e2tools $(DESTDIR)$(bindir)/e2cp
	$(LN_S) -f e2tools $(DESTDIR)$(bindir)/e2ln
	$(LN_S) -f e2tools $(DESTDIR)$(bindir)/e2ls
	$(LN_S) -f e2tools $(DESTDIR)$(bindir)/e2mkdir
	$(LN_S) -f e2tools $(DESTDIR)$(bindir)/e2mv
	$(LN_S) -f e2tools $(DESTDIR)$(bindir)/e2rm
	$(LN_S) -f e2tools $(DESTDIR)$(bindir)/e2tail

uninstall-hook:
	$(RM) -f $(DESTDIR)$(bindir)/e2cp
	$(RM) -f $(DESTDIR)$(bindir)/e2ln
	$(RM) -f $(DESTDIR)$(bindir)/e2ls
	$(RM) -f $(DESTDIR)$(bindir)/e2mkdir
	$(RM) -f $(DESTDIR)$(bindir)/e2mv
	$(RM) -f $(DESTDIR)$(bindir)/e2rm
	$(RM) -f $(DESTDIR)$(bindir)/e2tail

test:
	@echo "no tests yet"
