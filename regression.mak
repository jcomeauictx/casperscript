SHELL := /bin/bash
DIST_GS := /usr/bin/gs
TEST_GS := bin/gs
FILES := $(wildcard examples/*.* examples/*/*.*)
OUT := $(addsuffix .pnm, $(patsubst examples/%,reference/%,$(FILES)))
TEST_OUT := $(addsuffix .pnm, $(patsubst examples/%,testing/%,$(FILES)))
COMPARE := $(addsuffix .diff, $(TEST_OUT))
all: $(OUT) $(TEST_OUT) $(COMPARE)
	@echo Complete >&2
reference/%.pnm: examples/%
	mkdir -p $(@D)
	-$(DIST_GS) -dNOPAUSE -dBATCH -sDEVICE=ppm -sOutputFile=$@ $<
testing/%.pnm: examples/%
	mkdir -p $(@D)
	-$(TEST_GS) -dNOPAUSE -dBATCH -sDEVICE=ppm -sOutputFile=$@ $<
testing/%.diff: reference/%
	-diff $< $(@:.diff=)
check:
	@echo reference: $(OUT)
	@echo regression test: $(TEST_OUT)
