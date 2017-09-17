ACLOCAL_AMFLAGS = -I m4 --install
AM_CFLAGS = ${DEPS_CFLAGS}
AM_LIBS = ${DEPS_LIBS}

bin_PROGRAMS = mcbenchmark
mcbenchmark_SOURCES = src/mcbenchmark.c src/cmdline.c
BUILT_SOURCES = src/cmdline.c
src/cmdline.c: src/mcbenchmark.ggo
	${GENGETOPT} --input=src/mcbenchmark.ggo --output-dir=src
