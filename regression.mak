SHELL := /bin/bash
FILES := $(wildcard examples/*.* examples/*/*.*)
OUT := $(patsubst examples/%,reference/%,$(FILES))
all:
	echo $(OUT)
