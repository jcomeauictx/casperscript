ARCH := $(shell uname -m)
CONFIG_ARGS ?= --with-gs=cs --with-x --prefix=$(HOME)
export XCFLAGS ?= -DUSE_DEVELOPMENT_INITFILES=1
ifeq ("$(wildcard $(ARCH).mak)","")
	DEFAULT := Makefile
else
	DEFAULT := $(ARCH).mak
endif
default: $(DEFAULT)
	make -f $<
Makefile: | configure
	./configure $(CONFIG_ARGS)
configure: | autogen.sh
	./autogen.sh $(CONFIG_ARGS)
%:	| $(DEFAULT)
	make -f $(DEFAULT) "$@"
