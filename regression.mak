SHELL := /bin/bash
DIST_GS := /usr/bin/gs
TEST_GS := bin/gs
FILES := $(wildcard examples/*.* examples/*/*.*)
OUT := $(addsuffix .%00d.pnm, $(patsubst examples/%,reference/%,$(FILES)))
TEST_OUT := $(addsuffix .%00d.pnm, $(patsubst examples/%,testing/%,$(FILES)))
COMPARE := $(addsuffix .diff, $(subst %00d,*,$(TEST_OUT)))
all: $(OUT) $(TEST_OUT) $(COMPARE)
	@echo Complete >&2
reference/%.%00d.pnm: examples/%
	mkdir -p $(@D)
	-$(DIST_GS) -dNOPAUSE -dBATCH -sDEVICE=pnm -sOutputFile=$@ $<
testing/%.%00d.pnm: examples/%
	mkdir -p $(@D)
	-$(TEST_GS) -dNOPAUSE -dBATCH -sDEVICE=pnm -sOutputFile=$@ $<
testing/%.diff: reference/%
	-diff $< $(@D)
check:
	@echo reference: $(OUT)
	@echo regression test: $(TEST_OUT)
	@echo compare: $(COMPARE)
