About e2tools
=============

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


e2cp
====

This program copies files to/from an e2fs filesystem from/to the machine's
native filesystem.

Usage:
------

    e2cp [-0apv][-P mode][-O uid][-G gid][-d dest_dir][-s src_dir][file1..N dest]

    -0      Input lines terminated by a null character
    -a      Absolute directory names - create directories instead of just copying
            into the destination.  Only valid for copying into an ext2fs filesystem
    -d      Destination of files to be copied.  May be in the ext2fs filesystem or
            the host filesystem.
    -G      Set the default group to gid.
    -O      Set the default file owner to uid.
    -p      Preserve host file attributes (permissions, times, etc.) when copying
            files.
    -P      Set the file permissions (ie 755, 666).
    -s      The source of the files to be copied.
    -v      Be verbose.

    A `-` by itself means to use standard input/output

Examples:
---------

Copy a file and use the default permission and ownership of the current user:

    e2cp README.txt /tmp/boot.img:/tmp

Do the same thing, but keep permissions & ownership:

    e2cp -p README.txt /tmp/boot.img:/tmp

Dump a file to standard out:

    e2cp /tmp/boot.img:/tmp/README.txt - | wc

Get a file from standard input and put it on an unmounted partition:

    tar cf - /data/logs|gzip| e2cp - /dev/hdb1:/backup/logs.tar.gz

Copy the files from one directory and put them under another directory on
an unmounted partition, keeping the original paths, permissions & ownership:

    find /data -type f |tee filelist | e2cp -apv -d /dev/hdb1:/mirror

Copy files from a directory on an unmounted partition to a local directory
(Note: this does not recreate the directories in the local filesystem,
yet).  The list of files are read from standard input:

    e2cp -pv -s /dev/hdb1:/mirror -d /data2 < filelist

Copy a file to a file system and make the group and ownership root

    e2cp -G 0 -O 0 myfile /tmp/boot.img/boot/rootfile


e2mv
====

This program moves or renames files on an ext2fs filesystem.

Usage:
------

    e2mv [-vfs] source1 [... sourceN] destination

    -f   Force the operation to remove any existing files
    -s   Swap names of the two files
    -v   Be verbose

    Note: The source files must be explicitly stated.  It does not use regular
    expressions, yet.

Examples:
---------

Rename a file:

    e2cp -v /tmp/boot.img:/boot/grub/grub.conf /boot/grub/grub.conf.old

If `/boot/grub/grub.conf.old` already exists and is not a directory, this
will fail.  Use `-f` to force removal.

Move a file to a different directory (`/data/processed`):

    e2cp -v /dev/hdb1:/data/input0.txt /data/processed

To swap the names of two files:

    e2cp -vs /tmp/boot.img:/boot/grub/grub.conf /boot/grub/grub.conf.bk

To swap two files and use a new name for the first one:

    e2cp -vs boot.img:/boot/grub/grub.conf /boot/grub/grub.c2 /boot/grub/grub.c1

`/boot/grub/grub.conf` is now `/boot/grub/grub.c1` and
`/boot/grub/grub.c2`   is now `/boot/grub/grub.conf` .


e2rm
====

This program removes files and directories on an ext2 filesystem.

Usage:
------

    e2rm [-vr] filesystem:filepath ...fileN

    -r  Recursively delete files if a directory name is givne
    -v  Be verbose

Note: This program expects to have a full ext2 file specification for each
argument.

Examples:
---------

Remove a file

    e2rm -v boot.img:/boot/grub/grub.c1 /boot/grub/grub.c2

Remove a directory and all the files

    e2rm -r boot.img:/boot/grub


e2mkdir
=======

This program creates a directory on an ext2 filesystem.  It behaves similar
to `mkdir -p`.

Usage:
------

    e2mkdir [-G gid][-O uid][-P mode][-v] filesystem:directory...dirN

    -G      Set the default group to gid.
    -O      Set the default file owner to uid.
    -P      Set the file permissions (ie 755, 666).
    -v      Be verbose.

Note: This program expects to have a full ext2 file specification for each
argument.

Examples:
---------

This creates the directory `/boot/grub` with on `boot.img` with the current
user and group ids:

    e2mkdir boot.img:/boot/grub

To override the default ownership and permissions:

    e2mkdir -O 100 -P 700 /dev/hdb1:/data/backup


e2ln
====

This program is used to create hard links on an ext2 filesystem.

Usage:
------

    e2ln [-vsf] source destination

    -f   Force the operation to remove any existing files
    -s   Create a symlink
    -v   Be verbose

Note: creating symlinks is not operational at this time.

Examples:
---------

This will remove `/data/process_status` if it exists and isn't already a hard
link to `/data/info/status`.

    e2ln -vf /dev/hdb1:/data/info/status /data/process_status


e2ls
====

This program is used to list files and directories on an ext2 filesystem.

Usage:
------

    e2ls [-acDfilnrt][-d dir] file_specification

    -a  Show hidden directories
    -c  Sort by creation time (must include -t when using -l)
    -d dir Open the ext2 filesystem specified by dir
    -D  Show deleted files bracketed with ><.
    -f  No sorting of the file list
    -i  Show the inodes (very useful for the -l option)
    -l  Long listing
    -n  Like -l, but list numeric user and group IDs
    -r  Reverse the sort order
    -t  Sort by time
    -Z  Show SELinux label

Note: Files deleted via `e2rm` sometimes will show up even without the `-D`
option.  It is being investigated.

Examples:
---------

    e2ls -a boot.img:.
    e2ls -l /dev/hdb1:/data


e2tail
======

This program implements a basic version of the tail command.

Usage:
------

    e2tail [-n num_lines][-fF][-s sleep_interval] file

    -n  The number of lines to display
    -f  Output appended data as the file grows.  This is inode dependent, so if
        the file is renamed, it will keep checking it.
    -F  Output appended data as the file grows.  This is file name dependent,
        so if the file is renamed, it will check on any new files with the same
        name as the original.  This is useful for watching log files that may
        be rotated out occasionally.  This was requested by a person in the
        computer security field for monitoring 'honeypot' type machines.
    -s  The number of seconds to sleep before checking if the file has grown
        while in 'follow' mode.  The default is 1.

Examples:
---------

Check `/var/log/messages` on `/dev/sdc1` every 10 seconds

    e2tail -s 10 -F /dev/sdc1:/var/log/messages

