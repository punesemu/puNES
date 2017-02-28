SUBDIRS = misc src

ACLOCAL_AMFLAGS = -I m4

all: pre-build

if WITH_GIT
pre-build:
	echo "#define COMPILED \"`date +%m%d%Y%H%M%S`"\" > compiled.h
	if test -d $(top_srcdir)/.git ; then \
	  echo "#define GIT_COUNT_COMMITS \"r`$(GIT) -C $(top_srcdir) rev-list --count master`"\" >> compiled.h; \
	  echo "#define GIT_LAST_COMMIT \"`$(GIT) -C $(top_srcdir) log --format="%h" -n1`"\" >> compiled.h; \
	  echo "#define GIT_LAST_COMMIT_HASH \"`$(GIT) -C $(top_srcdir) log --format="%H" -n1`"\" >> compiled.h; \
	fi
else
pre-build:
	@echo "#define COMPILED \"`date +%m%d%Y%H%M%S`"\" > compiled.h
endif

