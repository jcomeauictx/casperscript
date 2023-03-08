ARCH := $(shell uname -m)
ifeq ("$(wildcard $(ARCH).mak)","")
	DEFAULT := Makefile
else
	DEFAULT := $(ARCH).mak
endif
default: $(DEFAULT)
	make -f $<
%:	| $(DEFAULT)
	make -f $(DEFAULT) "$@"
Makefile: | configure
	./configure
configure: | autogen.sh
	./autogen.sh
