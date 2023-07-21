# allow bashisms in recipes
SHELL := /bin/bash
CASPER ?= 1
# review `install` recipe if using other CONFIG_ARGS
INSTALL_PREFIX ?= $(HOME)
CONFIG_ARGS ?= --with-x --prefix=$(INSTALL_PREFIX)
ARCH := $(shell uname -m)
XCFLAGS += -Ibase -Ipsi -Iobj -I.
GSNAME := gs
ifneq ($(strip $(CASPER)),)
CS_VERSION ?= $(shell git tag)
GSNAME := cs-$(CS_VERSION)
CONFIG_ARGS += --with-gs=$(GSNAME)
XCFLAGS += -DBUILD_CASPERSCRIPT
endif
XCFLAGS += -DSYSLOG_DEBUGGING
XCFLAGS += -DUSE_LIBREADLINE
# CHA prompt was an attempt to fix prompt on Termux, but it fails on xterm
XCFLAGS += -DUSE_CHA_PROMPT
#XCFLAGS += -DTEST_ZCASPER=1
CASPERLIBS += -lreadline
XTRALIBS += $(CASPERLIBS)
VDIFF_TESTDIRS := reference latest
VDIFF_TESTFILE ?= cjk/iso2022.ps.1.pnm
VDIFF_TESTFILES := $(foreach dir,$(VDIFF_TESTDIRS),$(dir)/$(VDIFF_TESTFILE))
export CASPER XTRALIBS
export VDIFF_TESTFILES GSNAME # for `make env` check
ifeq ("$(wildcard $(ARCH).mak)","")
	CS_DEFAULT := Makefile
	CS_MAKEFILES := $(CS_DEFAULT)
else
	CS_DEFAULT := $(ARCH).mak
	CS_MAKEFILES := $(CS_DEFAULT) Makefile
endif
all: $(CS_MAKEFILES)
	XCFLAGS="$(XCFLAGS)" $(MAKE) -f $<
	# trick for cstestcmd.cs test on unix-y systems
	if [ \! -e bin/cs.exe ]; then \
		cd bin && ln -s $(GSNAME) cs.exe; \
	fi
install: $(CS_MAKEFILES)
	make -f $< install
	# make other aliases
	if [ "$${GSNAME:0:2}" = cs ]; then \
	 cd $(INSTALL_PREFIX)/bin && ln -sf $(GSNAME) cs && ln -sf cs gs; \
	else \
	 cd $(INSTALL_PREFIX)/bin && ln -sf gs cs; \
	fi
	# "console cs" for -dNODISPLAY
	cd $(INSTALL_PREFIX)/bin && ln -sf cs ccs
	# "batch console cs" for csbin/*
	cd $(INSTALL_PREFIX)/bin && ln -sf cs bccs
Makefile: | configure
	./configure $(CONFIG_ARGS)
configure: | autogen.sh
	./autogen.sh $(CONFIG_ARGS)
env:
	$@
%:	*/%.c
	[ "$<" ] || (echo Nothing to do >&2; false)
	$(MAKE) XCFLAGS="$(XCFLAGS)" $(<:.c=)
	mv $(<D)/$@ .
%:	| $(CS_MAKEFILES)
	$(MAKE) XCFLAGS="$(XCFLAGS)" -f $(CS_DEFAULT) "$@"
vdiff: vdiff.cs $(VDIFF_TESTFILES)
	./$^
/tmp/vdiff.pdf: vdiff.cs $(VDIFF_TESTFILES)
	bin/gs -dNOSAFER -sDEVICE=pdfwrite -sOutputFile=$@ \
	 -sPAPERSIZE=ledger -C -- $^ 1
tests:
	make -f regression.mak
help:
	bin/gs -h
ghostscript:
	-$(MAKE) distclean
	$(MAKE) CASPER= all install
