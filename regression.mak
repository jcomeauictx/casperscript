SHELL := /bin/bash
DIST_GS := /usr/bin/gs
TEST_GS := bin/gs
FILES := $(wildcard examples/*.* examples/*/*.*)
OUT := $(addsuffix .1.pnm, $(patsubst examples/%,reference/%,$(FILES)))
TEST_OUT := $(addsuffix .1.pnm, $(patsubst examples/%,testing/%,$(FILES)))
COMPARE := $(addsuffix .diff, $(TEST_OUT))
TIMESTAMP := $(shell date '+%Y-%m-%d %H:%M:%S')
LOG := tests.log
all: init $(OUT) $(TEST_OUT) $(COMPARE)
	@echo Complete >&2
init:
	@echo gs regression tests output $(TIMESTAMP) > $(LOG)
reference/%.1.pnm: examples/%
	mkdir -p $(@D)
	-$(DIST_GS) -dNOPAUSE -dBATCH -sDEVICE=pnm \
		-sOutputFile=$(@:.1.pnm=.%0d.pnm) $<
testing/%.1.pnm: examples/%
	mkdir -p $(@D)
	-$(TEST_GS) -dNOPAUSE -dBATCH -sDEVICE=pnm \
		-sOutputFile=$(@:.1.pnm=.%0d.pnm) $<
testing/%.diff: reference/%
	-for file in $(<:.1.pnm=*); do \
		echo comparing $$file >> $(LOG); \
		diff $$file $(@D) 2>&1 | tee -a $(LOG); \
	done
check:
	@echo reference: $(OUT)
	@echo regression test: $(TEST_OUT)
	@echo compare: $(COMPARE)
