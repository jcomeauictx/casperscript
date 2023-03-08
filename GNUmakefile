default: Makefile
	make -f $<
%:	| Makefile
	make -f Makefile "$@"
Makefile: | configure
	./configure
configure: | autogen.sh
	./autogen.sh
