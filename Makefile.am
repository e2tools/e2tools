########################################################################
# autotools control definitions
########################################################################

ACLOCAL_AMFLAGS = -I m4 -I e2t-m4


########################################################################
# e2tools variables to append to
########################################################################

# hook targets
all_targets =
clean_targets =
install_targets =
uninstall_targets =
phony_targets =

# list of e2tools like e2ls and e2cp etc
e2tools_list =

# list of -e 's|||g' arguments to sed for preprocessing *.in files
sed_in_commands =


########################################################################
# Automake variables to append to
########################################################################

BUILT_SOURCES =
CLEANFILES =
EXTRA_DIST =
SUFFIXES =
TESTS =

bin_PROGRAMS =
dist_man_MANS =
man_MANS =


########################################################################
# Documentation files
########################################################################

EXTRA_DIST += DEVELOPMENT.md
EXTRA_DIST += NEWS.md
EXTRA_DIST += README.md
EXTRA_DIST += TODO


########################################################################
# include files
########################################################################

include src/include.am
include tests/include.am


########################################################################
# .in replacement rules
########################################################################

HASHMARK = \#

if HAVE_EXT2FS_XATTRS
sed_in_commands += -e 's|[@]HAVE_EXT2FS_XATTRS_MAN_FALSE@||g'
sed_in_commands += -e 's|[@]HAVE_EXT2FS_XATTRS_MAN_TRUE@|.\\" |g'
sed_in_commands += -e 's|[@]HAVE_EXT2FS_XATTRS_FALSE@||g'
sed_in_commands += -e 's|[@]HAVE_EXT2FS_XATTRS_TRUE@|$(HASHMARK) |g'
else
sed_in_commands += -e 's|[@]HAVE_EXT2FS_XATTRS_MAN_FALSE@|.\\" |g'
sed_in_commands += -e 's|[@]HAVE_EXT2FS_XATTRS_MAN_TRUE@||g'
sed_in_commands += -e 's|[@]HAVE_EXT2FS_XATTRS_FALSE@|$(HASHMARK) |g'
sed_in_commands += -e 's|[@]HAVE_EXT2FS_XATTRS_TRUE@||g'
endif

sed_in_commands += -e 's|[@]top_builddir@|$(top_builddir)|g'
sed_in_commands += -e 's|[@]top_srcdir@|$(top_srcdir)|g'
sed_in_commands += -e 's|[@]DD@|$(DD)|g'
sed_in_commands += -e 's|[@]MKE2FS@|$(MKE2FS)|g'

SUFFIXES += .1.in .1
.1.in.1:
	$(MKDIR_P) $(@D)
	$(SED) $(sed_in_commands) < $< > $@

SUFFIXES += .sh.in .sh
.sh.in.sh:
	$(MKDIR_P) $(@D)
	$(SED) $(sed_in_commands) < $< > $@
	$(CHMOD) +x $@


########################################################################
# local hooks we have collected targets in variables for
########################################################################

.PHONY: $(phony_targets)

all-local: $(all_targets)

install-exec-hook: $(install_targets)

uninstall-hook: $(uninstall_targets)

clean-local: $(clean_targets)
	-rm -f *.i *.s


########################################################################
# End of Makefile.am
########################################################################
