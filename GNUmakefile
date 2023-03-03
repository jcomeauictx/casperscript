default: Makefile
	make -f $<
%:	| Makefile
	make -f Makefile "$@"
Makefile: | autogen.sh
	./autogen.sh
