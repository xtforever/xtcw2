# cookbook/Makefile — convenience wrapper around cookbook/scripts/.
#
#   make build-debug        build xtcw2 + app, debug mode
#   make build-production   build xtcw2 + app, production mode (no MLS_DEBUG)
#   make smoke-debug        start smoke test for the debug binary
#   make smoke-production   start smoke test for the production binary
#   make test               build + smoke-test both modes, archive to dist/
#   make run                run the app against the current build

SCRIPTS := $(CURDIR)/scripts

.PHONY: build-debug build-production smoke-debug smoke-production test run

build-debug:
	$(SCRIPTS)/build.sh debug

build-production:
	$(SCRIPTS)/build.sh production

smoke-debug:
	$(SCRIPTS)/smoke-test.sh debug

smoke-production:
	$(SCRIPTS)/smoke-test.sh production

test:
	$(SCRIPTS)/test-all.sh

run:
	$(SCRIPTS)/run.sh
