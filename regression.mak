SHELL := /bin/bash
FILES := $(wildcard examples/*.* examples/*/*.*)
OUT := $(addsuffix .pnm, $(patsubst examples/%,reference/%,$(FILES)))
all:
	echo $(OUT)
