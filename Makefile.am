ACLOCAL_AMFLAGS = -I m4 --install

bin_PROGRAMS = mcbenchmark
mcbenchmark_SOURCES = src/mcbenchmark.c src/cmdline.c
mcbenchmark_CFLAGS = $(MEMCACHED_CFLAGS)
mcbenchmark_LDFLAGS = $(MEMCACHED_LIBS)


BUILT_SOURCES = src/cmdline.c
CLEANFILES = src/cmdline.c
src/cmdline.c: src/mcbenchmark.ggo
	$(GENGETOPT) --input=src/mcbenchmark.ggo --output-dir=src
