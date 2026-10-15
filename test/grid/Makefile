include $(shell echo $${PREFIX-/usr})/share/smartmet/devel/makefile.inc

# Examples (default: run smartmet-plugin-test directly)
#    TEST_RUNNER="gdb --args"
#    TEST_RUNNER="valgrind --log-file=valgrind.out --leak-check=full --leak-resolution=high"
TEST_RUNNER ?=
TEST_TIMEOUT ?= 300

test test-grid:	start-valkey-db
	if $(MAKE) test-impl; then ok=true; else ok=false; fi; $(MAKE) stop-valkey-db; $$ok

test-impl:
	$(TEST_RUNNER) smartmet-plugin-test --handler /wfs \
		--reactor-config cnf/reactor.conf \
		--failures-dir failures \
		--timeout $(TEST_TIMEOUT)

clean:
	rm -rf failures
	rm -rf cnf/grid

start-valkey-db:
	rm -rf cnf/grid
	cp -pr $(datadir)/smartmet/test/grid cnf/
	@echo "*** Creating the Redis configuration file."
	smartmet-grid-test-config-creator cnf/environment.conf cnf/grid/redis/redis-template.conf cnf/grid/redis/redis.conf
	@echo "*** Starting the Valkey database."
	@valkey-server cnf/grid/redis/redis.conf & echo $$! > cnf/grid/redis/valkey.pid

stop-valkey-db:
	@if [ -f cnf/grid/redis/valkey.pid ] ; then \
        echo "*** Stopping the Valkey database."; \
		cat cnf/grid/redis/valkey.pid | xargs kill -9 2>/dev/null; \
		rm -f cnf/grid/redis/valkey.pid; \
	fi
