SUBDIRS = lib .
ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = e2cp

AM_CPPFLAGS = $(EXT2FS_CFLAGS) $(COM_ERR_CFLAGS)
e2cp_LDADD = $(EXT2FS_LIBS) $(COM_ERR_LIBS)
e2cp_SOURCES = \
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

EXTRA_DIST = README TODO e2tools.spec

install-exec-hook:
	$(LN_S) -f e2cp $(DESTDIR)$(bindir)/e2ln
	$(LN_S) -f e2cp $(DESTDIR)$(bindir)/e2ls
	$(LN_S) -f e2cp $(DESTDIR)$(bindir)/e2mkdir
	$(LN_S) -f e2cp $(DESTDIR)$(bindir)/e2mv
	$(LN_S) -f e2cp $(DESTDIR)$(bindir)/e2rm
	$(LN_S) -f e2cp $(DESTDIR)$(bindir)/e2tail

uninstall-hook:
	$(RM) -f $(DESTDIR)$(bindir)/e2ln
	$(RM) -f $(DESTDIR)$(bindir)/e2ls
	$(RM) -f $(DESTDIR)$(bindir)/e2mkdir
	$(RM) -f $(DESTDIR)$(bindir)/e2mv
	$(RM) -f $(DESTDIR)$(bindir)/e2rm
	$(RM) -f $(DESTDIR)$(bindir)/e2tail
