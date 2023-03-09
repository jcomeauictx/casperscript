export XCFLAGS += -DPNG_ARM_NEON_OPT=0
all:
	$(MAKE) -f Makefile
env:
	$@
%:
	$(MAKE) -f Makefile "$@"
