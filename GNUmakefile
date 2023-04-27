ARCH := $(shell uname -m)
CONFIG_ARGS ?= --with-gs=cs --with-x --prefix=$(HOME)
XCFLAGS += -Ibase -Ipsi -Iobj -I.
XCFLAGS += -DBUILD_CASPERSCRIPT
XCFLAGS += -DSYSLOG_DEBUGGING
XCFLAGS += -DUSE_LIBREADLINE
#XCFLAGS += -DTEST_ZCASPER=1
CASPERLIBS += -lreadline
XTRALIBS += $(CASPERLIBS)
export XTRALIBS
ifeq ("$(wildcard $(ARCH).mak)","")
	CS_DEFAULT := Makefile
	CS_MAKEFILES := $(CS_DEFAULT)
else
	CS_DEFAULT := $(ARCH).mak
	CS_MAKEFILES := $(CS_DEFAULT) Makefile
endif
default: $(CS_MAKEFILES)
	XCFLAGS="$(XCFLAGS)" $(MAKE) -f $<
	# trick for cstestcmd.cs test on unix-y systems
	if [ \! -e bin/cs.exe ]; then \
		cd bin && ln -s cs cs.exe; \
	fi
	# make other aliases
	cd bin && ln -sf cs gs
	cd bin && ln -sf cs ccs  # "console cs" for -dNODISPLAY
	cd bin && ln -sf cs bccs  # "batch console cs" for csbin/*
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
vdiff: vdiff.cs
	./$< reference/cjk/iso2022.ps.1.pnm testing/cjk/iso2022.ps.1.pnm
/tmp/vdiff.pdf: vdiff.cs
	bin/gs -dNOSAFER -sDEVICE=pdfwrite -sOutputFile=$@ \
	 -sPAPERSIZE=ledger -C -- $< \
	 reference/cjk/iso2022.ps.1.pnm \
	 testing/cjk/iso2022.ps.1.pnm 1
