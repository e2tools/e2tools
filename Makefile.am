INCLUDES = -I@includedir@

.c.o:
	$(COMPILE) -c $<

bin_PROGRAMS = e2cp
e2cp_SOURCES = copy.c \
	e2tools.c e2tools.h \
	elist.c elist.h \
	ln.c \
	ls.c \
	mkdir.c \
	mv.c \
	progress.c \
	read.c \
	rm.c \
	tail.c \
	util.c \
	write.c
e2cp_LDADD = -L@libdir@ -lext2fs -lcom_err
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
