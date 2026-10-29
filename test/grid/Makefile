include $(or $(PREFIX),$(shell smartbuildcfg --prefix))/share/smartmet/devel/makefile.inc

# Examples (default: run smartmet-plugin-test directly)
#    TEST_RUNNER="gdb --args"
#    TEST_RUNNER="valgrind --log-file=valgrind.out --leak-check=full --leak-resolution=high"
TEST_RUNNER ?=
TEST_TIMEOUT ?= 300

# Starts the Redis test database listening only on a Unix socket below /tmp
GRID_TEST_REDIS ?= smartmet-grid-test-redis

test test-grid:	start-valkey-db
	if $(MAKE) test-impl; then ok=true; else ok=false; fi; $(MAKE) stop-valkey-db; $$ok

test-impl:
	$(TEST_RUNNER) smartmet-plugin-test --handler /wfs \
		--reactor-config cnf/reactor.conf \
		--failures-dir failures \
		--timeout $(TEST_TIMEOUT)

clean:
	rm -rf failures
	-$(GRID_TEST_REDIS) stop
	rm -rf cnf/grid

start-valkey-db:
	$(GRID_TEST_REDIS) start

stop-valkey-db:
	$(GRID_TEST_REDIS) stop
