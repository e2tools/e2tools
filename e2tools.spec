Summary: Tools to access files on an unmounted ext2 filesystem
Name: e2tools
Version: 0.0.16
Release: 1
Copyright: GPL
Group: Applications/filesystems
Source0: http://ksheff.net/sw/e2tools/%{name}-%{version}.tar.gz
Patch1: e2tools.diff
BuildRoot: /var/tmp/%{name}-root
Prefix: /usr
Packager: Keith W. Sheffield <sheff@pobox.com>
Requires: e2fsprogs > 1.25
BuildRequires: e2fsprogs-devel > 1.25

%description
E2tools is a simple set of GPL'ed utilities to read, write, and manipulate
files in an ext2/ext3 filesystem. I wrote these tools in order to copy files
into a linux filesystem on a machine that does not have ext2 support. Of
course, they can also be used on a linux machine to read/write to disk images
or floppies without having to mount them.


%prep
%setup

chmod 755 configure
autoconf

%build
./configure --prefix=$RPM_BUILD_ROOT/usr
make
%install
rm -rf $RPM_BUILD_ROOT
make install
mv -f $RPM_BUILD_ROOT/usr/bin $RPM_BUILD_ROOT/usr/sbin

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%attr(755,root,root)       /usr/sbin/
%doc README TODO COPYING ChangeLog

%changelog
* Tue Apr 06 2004 Keith Sheffield
- Updated version due to bug fix release.
* Wed Jul 09 2003 Keith Sheffield
- autoconfiscated the build system
* Thu Aug 08 2002 Ralf Spenneberg <ralf@spenneberg.de>
- first release

