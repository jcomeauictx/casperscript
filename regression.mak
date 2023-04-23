SHELL := /bin/bash
DIST_GS := /usr/bin/gs
TEST_GS := bin/gs
FILES := $(wildcard examples/*.* examples/*/*.*)
OUT := $(addsuffix .1.pnm, $(patsubst examples/%,reference/%,$(FILES)))
TEST_OUT := $(addsuffix .1.pnm, $(patsubst examples/%,testing/%,$(FILES)))
COMPARE := $(addsuffix .diff, $(TEST_OUT))
all: $(OUT) $(TEST_OUT) $(COMPARE)
	@echo Complete >&2
reference/%.1.pnm: examples/%
	mkdir -p $(@D)
	-$(DIST_GS) -dNOPAUSE -dBATCH -sDEVICE=pnm \
		-sOutputFile=$(@D)/$*.%00d.pnm $<
testing/%.1.pnm: examples/%
	mkdir -p $(@D)
	-$(TEST_GS) -dNOPAUSE -dBATCH -sDEVICE=pnm
		-sOutputFile=$(@D)/$*.%00d.pnm $<
testing/%.diff: reference/%
	-diff $(<:.1.pnm=*) $(@D) | tee -a tests.log
check:
	@echo reference: $(OUT)
	@echo regression test: $(TEST_OUT)
	@echo compare: $(COMPARE)
