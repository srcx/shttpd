# shttpd

**Warning: unmaintained since 2009!**

Motto: 's' as stupid.

## Compilation and installation

Procedure is simple. Unpack distribution archive, switch to directory shttpd
and enter these commands on the command line:
```
./configure
make
```
This procedure is intended mainly for systems compatible with Unix. It can
work with packages allowing compilation of Unix programs like DJGPP, Cygwin
and mingw32, but this was not tried. Other systems may require larger
changes. If make chokes on a Makefile syntax, try running GNU make (gmake).

Configure can have these additional paramaters (for more see ./configure
--help):
```
--without-pthreads	do not use threads (else their presence is checked)
			- use if compilation failes with active threads
```
Make can have some from the following parameters:
```
all (or nothing)	compiles whole shttpd
clean			deletes files generated during compilation which
                        are not needed for shttpd's run
cleanall		deletes all files generated during compilation
distclean		deletes all files generated during compilation and
                        configuration
```
Name of resulting file is shttpd (or with some extension - depends on the
system). It is prepared for test run immediately after compilation (for this
you need files shttpd.cfg, mime.types and directory html, during its run
files access_log.shttpd and error_log.shttpd are created). Installation of
these files must be done manually.

Documentation is in the HTML format and is placed in directory doc/.
