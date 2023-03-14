ARCH := $(shell uname -m)
CONFIG_ARGS ?= --with-gs=cs --with-x --prefix=$(HOME)
export XCFLAGS ?= -DUSE_DEVELOPMENT_INITFILES=1
ifeq ("$(wildcard $(ARCH).mak)","")
	CS_DEFAULT := Makefile
	CS_MAKEFILES := $(CS_DEFAULT)
else
	CS_DEFAULT := $(ARCH).mak
	CS_MAKEFILES := $(CS_DEFAULT) Makefile
endif
default: $(CS_MAKEFILES)
	make -f $<
	cd bin && ln -s cs cs.exe  # for cstestcmd.cs, will fail on Windows
Makefile: | configure
	./configure $(CONFIG_ARGS)
configure: | autogen.sh
	./autogen.sh $(CONFIG_ARGS)
%:	| $(CS_MAKEFILES)
	make -f $(CS_DEFAULT) "$@"
