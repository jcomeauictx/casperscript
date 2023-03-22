ARCH := $(shell uname -m)
CONFIG_ARGS ?= --with-gs=cs --with-x --prefix=$(HOME)
XCFLAGS += -DUSE_DEVELOPMENT_INITFILES=1
#XCFLAGS += -DSYSLOG_DEBUGGING=1
#XCFLAGS += -DTEST_ZCASPER=1
export XCFLAGS
ifeq ("$(wildcard $(ARCH).mak)","")
	CS_DEFAULT := Makefile
	CS_MAKEFILES := $(CS_DEFAULT)
else
	CS_DEFAULT := $(ARCH).mak
	CS_MAKEFILES := $(CS_DEFAULT) Makefile
endif
default: $(CS_MAKEFILES)
	make -f $<
	# trick for cstestcmd.cs test on unix-y systems
	if [ \! -e bin/cs.exe ]; then \
		cd bin && ln -s cs cs.exe; \
	fi
Makefile: | configure
	./configure $(CONFIG_ARGS)
configure: | autogen.sh
	./autogen.sh $(CONFIG_ARGS)
env:
	$@
%:	*/%.c
	[ "$<" ] || (echo Nothing to do >&2; false)
	make $(<:.c=)
	mv $(<D)/$@ .
%:	| $(CS_MAKEFILES)
	make -f $(CS_DEFAULT) "$@"
