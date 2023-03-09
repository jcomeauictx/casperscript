ARCH := $(shell uname -m)
export XCFLAGS ?= -DUSE_DEVELOPMENT_INITFILES=1
ifeq ("$(wildcard $(ARCH).mak)","")
	DEFAULT := Makefile
else
	DEFAULT := $(ARCH).mak
endif
default: $(DEFAULT)
	make -f $<
Makefile: | configure
	./configure
configure: | autogen.sh
	./autogen.sh
%:	| $(DEFAULT)
	make -f $(DEFAULT) "$@"
