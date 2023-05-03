SHELL := /bin/bash
DIST_GS := /usr/bin/gs
TEST_GS := bin/gs
FILES := $(wildcard examples/*.* examples/*/*.*)
OUT := $(addsuffix .1.pnm, $(patsubst examples/%,reference/%,$(FILES)))
TEST_OUT := $(addsuffix .1.pnm, $(patsubst examples/%,latest/%,$(FILES)))
COMPARE := $(addsuffix .diff, $(TEST_OUT))
TIMESTAMP := $(shell date '+%Y-%m-%d %H:%M:%S')
LOG := tests.log
# fontpaths for Debian, likely needed for regression tests
# leave a space as shown, we will replace it with ':'
NULLSTR :=
SPACE := $(NULLSTR) $(NULLSTR)
PATHSEP := :
TESTSPACE := A$(SPACE)B
GS_FONTPATH += /usr/share/ghostscript/fonts
GS_FONTPATH += /var/lib/ghostscript/fonts
GS_FONTPATH += /usr/share/cups/fonts
GS_FONTPATH += /usr/local/lib/ghostscript/fonts
GS_FONTPATH += /usr/share/fonts
GS_FONTPATH := $(subst $(SPACE),$(PATHSEP),$(GS_FONTPATH))
export GS_FONTPATH
all: init $(OUT) $(TEST_OUT) $(COMPARE)
	@echo Complete >&2
init:
	@echo gs regression tests output $(TIMESTAMP) > $(LOG)
	@echo gs regression tests output $(TIMESTAMP) > stdout_reference_$(LOG)
	@echo gs regression tests output $(TIMESTAMP) > stderr_reference_$(LOG)
	@echo gs regression tests output $(TIMESTAMP) > stdout_latest_$(LOG)
	@echo gs regression tests output $(TIMESTAMP) > stderr_latest_$(LOG)
reference/%.1.pnm: examples/%
	mkdir -p $(@D)
	-$(DIST_GS) -dNOPAUSE -dBATCH -sDEVICE=pnm -dALLOWPSTRANSPARENCY \
		-sOutputFile=$(@:.1.pnm=.%0d.pnm) $< \
		>>stdout_reference_$(LOG) 2>>stderr_reference_$(LOG)
latest/%.1.pnm: examples/%
	mkdir -p $(@D)
	-$(TEST_GS) -dNOPAUSE -dBATCH -sDEVICE=pnm -dALLOWPSTRANSPARENCY \
		-sOutputFile=$(@:.1.pnm=.%0d.pnm) $< \
		>>stdout_latest_$(LOG) 2>>stderr_latest_$(LOG)
latest/%.diff: reference/%
	-for file in $(<:.1.pnm=*); do \
		echo comparing $$file >> $(LOG); \
		diff $$file $(@D) 2>&1 | tee -a $(LOG); \
	done
check:
	@echo reference: $(OUT)
	@echo regression test: $(TEST_OUT)
	@echo compare: $(COMPARE)
env:
	$@
vdiffs/%.pdf: reference/% latest/%
	mkdir -p $(@D)
	bin/gs -dNOSAFER -dNOPAUSE -dBATCH -sDEVICE=pdfwrite -sOutputFile=$@ \
	 -sPAPERSIZE=ledger -C -- vdiff.cs $^ 1
