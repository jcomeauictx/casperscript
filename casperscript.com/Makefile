SERVER := $(notdir $(PWD))
DOCROOT := /var/www/$(SERVER)
DRYRUN ?= --dry-run

upload:
	rsync -avuz \
	 $(DRYRUN) \
	 --exclude=.git \
	 --exclude=Makefile \
	 --exclude=README.md \
	 . $(SERVER):$(DOCROOT)/
