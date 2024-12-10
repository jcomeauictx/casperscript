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
GSBUILDNAME ?= $(shell awk -F= '$$1 == "GS" {print $$2}' config.log | tr -d "'")
GS_BUILDVERSION := $(shell string=$(GSBUILDNAME); echo $${string##*-})
CONFIG_ARGS += --with-gs=$(GSNAME)
# for some reason, although configure.ac has `--disable-pixarlog`, it is not reliable
GS_TIFF_CONFIGURE_OPTS += --disable-pixarlog
# *** No rule to make target 'obj/libocr_0_@OCR_SHARED@.dev', needed by 'obj/libocr.dev'.  Stop.
CONFIG_ARGS += --without-tesseract
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
TESTCASPER ?= 0.0 cvbool =
CASPERTEST ?= (Resource/Init/casperscript.ps) run casper $(TESTCASPER)
CSBIN ?= $(wildcard csbin/*)
ifeq ($(SHOWENV),)
export CASPER XTRALIBS GS_TIFF_CONFIGURE_OPTS
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
	@echo building casperscript version $(CS_VERSION)
	[ -e "$<" ] || $(MAKE) $<
	XCFLAGS="$(XCFLAGS)" $(MAKE) -f $< 2>&1 | tee make.log
	# trick for cstestcmd.cs test on unix-y systems
	cd bin && ln -sf $(GSNAME) cs.exe
	# make the same symlinks as for install, but in $(CWD)/bin
	# NOTE: we're counting on GSNAME being unique per build!
	cd bin && for symlink in cs gs ccs bccs; do \
	 [ -f $$symlink-$(CS_VERSION) ] || \
	  ln -sf $(GSNAME) $$symlink-$(CS_VERSION); \
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
	 [ -f $$symlink-$(CS_VERSION) ] || \
	  ln -sf $(GSNAME) $$symlink-$(CS_VERSION); \
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
vdiff: vdiff.cs $(VDIFF_TESTFILES)
	./$^
/tmp/vdiff.pdf: vdiff.cs $(VDIFF_TESTFILES)
	bin/gs -dNOSAFER -sDEVICE=pdfwrite -sOutputFile=$@ \
	 -sPAPERSIZE=ledger -C -- $^ 1
test_csbin: $(CSBIN)
	for binfile in $+; do \
	 ./$$binfile one two three; \
	done
tests:
	make -f regression.mak
help:
	bin/gs -h
ghostscript:
	-$(MAKE) distclean
	$(MAKE) CASPER= all install
gscasper:
	$(GSCASPER)
caspertest:
	echo '$(CASPERTEST)' | $(GSCASPER)
remake: # unlike `rebuild`, just uses last build's version number
	XCFLAGS="-DDEBUG" $(MAKE) CS_VERSION=$(GS_BUILDVERSION)
rebuild: remake caspertest
	# if we changed anything, make sure we commit it before rebuild
	git diff --name-only --exit-code || \
	 (echo 'commit changes before `make rebuild`' >&2; false)
	$(MAKE) distclean all install  # after all that, we can rebuild
push:
	git push  # to default repository
	git push githost  # to personal repository
test_splitargs:
	XCFLAGS=-DTEST_SPLITARGS=1 $(MAKE) splitargs
	./splitargs -option0 -option1 -- arg0 arg1
	./splitargs -option0 -option1 -option2
	./splitargs -option0 --
	./splitargs -option - arg1
	@echo 'Must `make distclean` before attempting new build' >&2
%:	*/%.c
	[ "$<" ] || (echo Nothing to do >&2; false)
	$(MAKE) XCFLAGS="$(XCFLAGS)" $(<:.c=)
	mv $(<D)/$@ .
	rm -f $(<:.c=.o)
%:	| $(CS_MAKEFILES)
	$(MAKE) XCFLAGS="$(XCFLAGS)" -f $(CS_DEFAULT) "$@"
