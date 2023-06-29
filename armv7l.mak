XCFLAGS += -DARM_CASPER
XCFLAGS += -DPNG_ARM_NEON_OPT=0
export XCFLAGS
all:
	$(MAKE) -f Makefile
env:
	$@
%:
	$(MAKE) -f Makefile "$@"
