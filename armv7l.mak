all:
	$(MAKE) XCFLAGS=-DPNG_ARM_NEON_OPT=0 -f Makefile
%:
	$(MAKE) XCFLAGS=-DPNG_ARM_NEON_OPT=0 -f Makefile $@
env:
	$@
