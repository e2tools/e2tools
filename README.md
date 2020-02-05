About e2tools
=============

[![master branch build status](https://api.travis-ci.org/e2tools/e2tools.svg?branch=master)](https://travis-ci.org/e2tools/e2tools)

e2tools is a simple set of GPL'ed utilities to read, write, and
manipulate files in an ext2/ext3 filesystem.  These utilities access a
filesystem directly using the ext2fs library.  I wrote these tools in order
to copy files into a linux filesystem on a machine that does not have ext2
support.  Of course, they can also be used on a Linux machine to read/write
to disk images or floppies without having to mount them or have root
access.

Supported functionality:

  * copy files: `e2cp`
  * move files: `e2mv`
  * remove files: `e2rm`
  * create directory: `e2mkdir`
  * create hard links: `e2ln`
  * list files/directories: `e2ls`
  * output the last part of a file: `e2tail`

Requirements:

  * e2fsprogs-1.27 or later - http://e2fsprogs.sourceforge.net/
  * gcc - e2fsprogs will not compile with some proprietary compilers (ie SCO's)

For questions, suggestions, or remarks please open an issue at

     https://github.com/e2tools/e2tools/issues

For patches, please create a pull request at

     https://github.com/e2tools/e2tools/pulls

or open an issue with a patch attached.


Quickstart Documentation
========================

In general, to specify a directory or file on an ext2 filesystem for the
e2tools utilities, use the following form:

    filesystem:directory_path

The filesystem can be an unmounted partition or a regular file that's been
formatted to contain an ext2 filesystem.  In general, if a command takes
multiple file names on the command line, if the first one contains an ext2
file specification, the rest of the files are assumed to be on the same
filesystem until another one is explicitly stated:

    /tmp/boot.img:/tmp/file1 /tmp/file2 /tmp/file3 /tmp/boot2.img:/tmp/file4

Files 1-3 are on `/tmp/boot.img` and the last file is on `/tmp/boot2.img`

A few quick ideas:

    # create 16MB image file
    dd if=/dev/null of=test.img bs=1 count=1 seek=16M

	# create filesystem inside image file
	mkfs.ext2 -F test.img

    # create directory foo in root directory of filesystem image
    e2mkdir test.img:/foo

	# look into root directory of filesystem image
	e2ls -l test.img:

    # copy README.md into foo dir
    e2cp README.md test.img:/foo

	# look what is inside the foo dir
	e2ls -l test.img:/foo

	# remove file again
	e2rm test.img:/foo/README.md

