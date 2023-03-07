EXTRA := EXTRA_INIT_FILES="../Resource/Init/casperscript.ps"
default: Makefile
	make $(EXTRA) -f $<
%:	| Makefile
	make $(EXTRA) -f Makefile "$@"
Makefile: | configure
	./configure
configure: | autogen.sh
	./autogen.sh
