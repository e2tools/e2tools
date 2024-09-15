e2tools-0.1.2 (2024-09-16)
==========================

  * Make the build compatible with Automake 1.17's warning about hash
    characters in Makefile.am files.

  * Require at least C99 with stdint.h etc.

  * Documentation updates.


e2tools-0.1.0 (2020-02-07)
==========================

  * With Keith Sheffield's e2tools home page having disappeared,
    Christoph Gysin has invited Hans Ulrich Niedermann to join the
    efforts maintaining e2tools at https://github.com/e2tools/e2tools
    and make https://e2tools.github.io/ the new homepage (in early
    2020).

  * Merges the diverged Rudoy/Gysin and Niedermann branches.

  * `e2tail` now accepts the no-op `-v` option.

  * Enlarged another buffer to make sure the data fits.

  * Fix a few signed/unsigned integer comparisons in `e2tail` (found
    via compiler warnings).

  * Add SELinux support for `e2ls -Z`, which is built only if
    libext2fs actually contains the functions of the xattrs API.

  * Moved all e2tool documentation from the `README.md` including the
    tool examples to the respective man pages.

  * Added a first simple test script to run during `make check`
    checking the e2tools actually work.

  * `make dist` only generates gzip compressed `*.tar.gz` tarballs.

  * `e2ls` now aborts and exits with an error when it encounters an
    unrecognized commandline option.


e2tools-0.0.16.4 (early 2019, Niedermann branch only)
=====================================================

  * Fix compiler warnings on undefined functions and unused variables.

  * Fix one buffer size issue found by compiler warning.


e2tools without named releases (mid 2015, Rudoy/Gysin branch only)
==================================================================

  * Eugene Rudoy and Christoph Gysin clean up the code for use in
    Debian and other distributions.

  * Add e2tools executable to which all tools symlink (it used to be
    the e2cp executable which all other tools symlinked).

  * Pretty-print file mode in `e2ls` (`-rw-r--r--` and such).

  * Add `e2ls` option `-n` to show owner and group in numeric form.

  * `e2tail` stops accepting the no-op `-v` option.


e2tools-0.0.16.3 (late 2007)
============================

  * Hans Ulrich Niedermann's late 2007 build and compatibility cleanup
    efforts from https://github.com/ndim/e2tools mostly to help
    preparing the Fedora e2tools package.

  * Fix a few signed/unsigned integer comparisons (compiler warning).

  * Avoid assignment within expression (compiler warning).

  * This is basically the fork point where the Rudoy/Gysin branch on
    one side (2015) and the Niedermann branch (2019) on the other side
    started diverging.


e2tools-0.0.16 and earlier (the Keith Sheffield era)
====================================================

  * e2tools-0.0.16.tar.gz is the last tarball release from Keith
    Sheffield.  The e2tools capabilities have not changed
    significantly between that release and 2020.

  * 2004-04-06 is Keith Sheffield's last ChangeLog entry.

  * A e2tools-0.0.16.tar.gz link is on the e2tools page (wayback
    machine capture date 2004-06-09, "Last modified" 2004-04-06).

  * A e2tools-0.0.15.tar.gz link is on the e2tools page (wayback
    machine capture date 2004-02-05, "Last modified" 2003-07-12).

  * e2tools-0.0.13.tar.gz is the first release to be found on [Keith
    Sheffield's e2tools
    page](http://home.earthlink.net/~k_sheff/sw/e2tools/) using the
    [wayback machine](https://web.archive.org/) capture from
    2003-02-13.  The "Last modified" suggests that it has been
    released at 2002-08-08 or earlier, but the 2003-02-13 capture date
    at the very latest.
