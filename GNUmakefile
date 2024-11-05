# allow bashisms in recipes
SHELL := /bin/bash
CASPER ?= 1
# review `install` recipe if using other CONFIG_ARGS
INSTALL_PREFIX ?= /usr/local/casperscript
CONFIG_ARGS ?= --with-x --prefix=$(INSTALL_PREFIX)
ARCH := $(shell uname -m)
XCFLAGS += -Ibase -Ipsi -Iobj -I.
GSNAME := gs
ifneq ($(strip $(CASPER)),)
CS_VERSION ?= $(shell git rev-parse --short HEAD)
GSNAME := cs-$(CS_VERSION)
CONFIG_ARGS += --with-gs=$(GSNAME)
XCFLAGS += -DBUILD_CASPERSCRIPT -DINSTALL_PREFIX=$(INSTALL_PREFIX)
endif
XCFLAGS += -DSYSLOG_DEBUGGING
XCFLAGS += -DUSE_LIBREADLINE
# fix problem with off64_t after merge with artifex-ghostpdl
# https://stackoverflow.com/a/34853360/493161
XCFLAGS += -Doff64_t=__off64_t
# CHA prompt was an attempt to fix prompt on Termux, but it fails on xterm
XCFLAGS += -DUSE_CHA_PROMPT
#XCFLAGS += -DTEST_ZCASPER=1
CASPERLIBS += -lreadline
XTRALIBS += $(CASPERLIBS)
VDIFF_TESTDIRS := reference latest
VDIFF_TESTFILE ?= cjk/iso2022.ps.1.pnm
VDIFF_TESTFILES := $(foreach dir,$(VDIFF_TESTDIRS),$(dir)/$(VDIFF_TESTFILE))
GSCASPER := /usr/bin/gs -dNOSAFER -dNODISPLAY
CASPERTEST ?= (Resource/Init/casperscript.ps) run casper 0.0 cvbool =
ifeq ($(SHOWENV),)
export CASPER XTRALIBS
else
export
endif
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
	cd bin && ln -sf $(GSNAME) cs.exe
	# make the same symlinks as for install, but in $(CWD)/bin
	# NOTE: we're counting on GSNAME being unique per build!
	cd bin && for symlink in cs gs ccs bccs; do \
	 ln -sf $(GSNAME) $$symlink; \
	done
install: $(CS_MAKEFILES)
	make -f $< install
	# make other aliases:
	# gs "ghostscript"
	# cs "casperscript"
	# ccs "console cs (cs -dNODISPLAY)"
	# bccs "batch console cs"
	# NOTE: GSNAME is unique for each build, so overwrite all aliases
	cd $(INSTALL_PREFIX)/bin && for symlink in cs gs ccs bccs; do \
	 ln -sf $(GSNAME) $$symlink; \
	done
Makefile: | configure
	./configure $(CONFIG_ARGS)
configure: | autogen.sh
	./autogen.sh $(CONFIG_ARGS)
env:
ifeq ($(SHOWENV),)
	$(MAKE) SHOWENV=1 $@
else
	$@
endif
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
caspertest:
	echo '$(CASPERTEST)' | $(GSCASPER)
