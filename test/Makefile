CC = gcc
CFLAGS = -Wall -Werror -g -std=c99

PROG = tinyFsDemo
OBJS = tinyFsDemo.o

LIBTFS = libTinyFS.a
LIBTFS_OBJS = tfs_fs.o tfs_file.o
LIBTFS_SRCS = $(LIBTFS_OBJS:%.c=%.o)
LIBTFS_API_HDRS = tinyFS.h TinyFS_errno.h libTinyFS.h
LIBTFS_ALL_HDRS = $(LIBTFS_API_HDRS) libDisk.h tfs_internal.h

LIBDISK = libDisk.a
LIBDISK_OBJS = libDisk.o
LIBDISK_API_HDRS = tinyFS.h TinyFS_errno.h libDisk.h
LIBDISK_ALL_HDRS = $(LIBDISK_API_HDRS)

default: $(PROG)

# Demo program targets

$(PROG): $(OBJS) $(LIBDISK) $(LIBTFS)
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS) -L. -lTinyFS -lDisk

$(OBJS): tinyFsDemo.c $(LIBTFS_API_HDRS)
	$(CC) $(CFLAGS) -c $<

# libTFS.a targets

tfs_fs.o: tfs_fs.c $(LIBTFS_ALL_HDRS)
	$(CC) $(CFLAGS) -c $<

tfs_file.o: tfs_file.c $(LIBTFS_ALL_HDRS)
	$(CC) $(CFLAGS) -c $<

$(LIBTFS): $(LIBTFS_OBJS)
	ar r $@ $(LIBTFS_OBJS)
	ranlib $@

# libDisk.a targets

libDisk.o: libDisk.c $(LIBDISK_ALL_HDRS)
	$(CC) $(CFLAGS) -c -o $@ $<

$(LIBDISK): $(LIBDISK_OBJS)
	ar r $@ $(LIBDISK_OBJS)
	ranlib $@

# Misc targets

clean:
	rm -f $(OBJS) $(LIBDISK_OBJS) $(LIBTFS_OBJS)

clobber: clean
	rm -f $(PROG) $(LIBDISK) $(LIBTFS)

# You may need to modify the list of files added to the tar file.
# Check the contents of the resulting tgz file carefully!
submission:
	tar cf $@.tar *.c *.h Makefile
	gzip -9 $@.tar
