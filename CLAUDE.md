# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

The WFS plugin implements OGC Web Feature Service 2.0 for SmartMet Server. It provides an INSPIRE-compliant XML interface for meteorological and geographical feature data, built around a stored-query architecture where each query type has its own handler, configuration, and CTPP2 XML template.

## Build commands

```bash
make                    # Build wfs.so plugin + compiled templates
make test               # Run all tests (sqlite + oracle + postgresql + grid)
make test-sqlite        # Run tests with sqlite backend only (fastest, CI default)
make test-postgresql    # Run tests with PostgreSQL backend
make test-grid          # Run grid-specific tests
make format             # clang-format
make clean              # Clean all artifacts
make rpm                # Build RPM package
```

Tests are run from subdirectories under `test/` (base, real, grid). Each test subdirectory uses `smartmet-plugin-test` — an HTTP request/response comparison harness:

```bash
# Run a specific test subdirectory
make -C test/base test-sqlite

# Tests compare HTTP responses against expected output in test/<dir>/output/
# Failed tests go to test/<dir>/failures-<db_type>/
```

Test inputs live as `kvp/*.kvp` (GET parameters) and `xml/*.xml` (POST bodies), converted by Perl scripts (`MakeKVPGET.pl`, `MakeXMLPOST.pl`) into `input/` files that the test harness reads.

## Build output

- `libsmartmet-plugin-wfs.a` — static library from `libwfs/` sources
- `wfs.so` — shared plugin loaded by SmartMet Server (links the static lib + `wfs/` sources)
- `cnf/templates/*.c2t` — compiled CTPP2 templates (from `*.template` sources)

The shared object is checked at build time for unresolved symbols, excluding `SmartMet::Engine::`, `SmartMet::T::` and `SmartMet::QueryServer::` references (resolved at runtime by the server).

## Code layout

**`libwfs/`** — reusable WFS library (~18k LOC), the bulk of the implementation:
- `PluginImpl` — core request processing: parses KVP GET / XML POST, routes to request handlers
- `request/` — OGC WFS standard operations: `GetCapabilities`, `GetFeature`, `ListStoredQueries`, `DescribeStoredQueries`, `DescribeFeatureType`, `GetPropertyValue`
- `StoredQueryHandlerBase` — abstract base for all stored query handlers
- `StoredQueryMap` — registry of loaded handlers; `StoredQueryConfig` — handler configuration parser
- `RequestFactory` — creates request objects from KVP or XML input
- `XmlParser` — Xerces-C DOM parser with schema validation and grammar pool caching
- `Config` — plugin-level configuration (libconfig)

**`wfs/`** — concrete plugin implementation:
- `Plugin` — entry point class (registered with SmartMet Server)
- `stored_queries/` — concrete handler implementations, configured by stored query `.conf` files (examples in `cnf/` and `test/*/stored_queries/`)

**`cnf/`** — configuration and templates:
- `templates/` — CTPP2 templates for XML responses (compiled to `.c2t` at build time)
- `features/` — feature type definitions (name, namespace, CRS, bbox)
- `opendata_stored_queries_github/` — example stored query configurations
- `capabilities*.conf`, `XMLGrammarPool.dump`, `XMLSchemas.cache` — service metadata and XML validation data

Coordinate reference systems come from the GIS engine's CRS registry (the tests use `test/crs/`).

## Architecture: request flow

1. HTTP request hits `Plugin::requestHandler()` (or the language specific handler `<url>/<language>`), which delegates via `Plugin::realRequestHandler()` (selects the data `source`) to `PluginImpl::realRequestHandler()`
2. `RequestFactory` parses the request (KVP or XML) and creates the appropriate request object
3. For `GetFeature` (the main data request), the stored query ID routes to a handler from `StoredQueryMap`
4. The handler queries SmartMet engines, populates a `CTPP::CDT` hash, and renders XML via a compiled template
5. Response XML is optionally schema-validated and cached

## Stored query handler pattern

Handlers inherit from `StoredQueryHandlerBase` plus mixin classes for engine access and parameter handling:

- **Engine mixins**: `RequiresQEngine`, `RequiresObsEngine`, `RequiresGeoEngine`, `RequiresGridEngine`, `RequiresContourEngine`
- **Parameter mixins**: `SupportsTimeParameters`, `SupportsLocationParameters`, `SupportsBoundingBox`, `SupportsMeteoParameterOptions`, `SupportsTimeZone`, `SupportsQualityParameters`

Each stored query has a `.conf` file defining its stored query ID, constructor name, CTPP2 template, parameters with types/defaults, and handler-specific settings.

The `constructor_name` names a `StoredQueryHandlerFactoryDef` object which is looked up at runtime with `dlsym()` (`StoredQueryHandlerFactoryDef::construct`). The factory objects must therefore have external linkage: declare each one in `wfs/stored_queries/StoredQueryHandlerFactories.h`, and do not move them into anonymous namespaces or make them `static`.

## Key dependencies

**SmartMet engines** (loaded at runtime, not linked): QueryData, Observation, Geonames, Gis, Grid, Contour

**SmartMet libraries** (linked): spine, macgyver, newbase, timeseries, locus, grid-files

**External**: xerces-c + xqilla (XML parsing/XPath), GDAL, libconfig++, jsoncpp, libpqxx, ctpp2 (templates), Boost (serialization, thread, iostreams)

## Testing details

Tests use a fixed timestamp (`lockedTimeStamp = "2012-12-12T12:12:12Z"` in the test `wfs.conf`) for reproducibility. The `smartmet-plugin-test` harness starts a SmartMet Server instance, sends requests, and diffs responses against expected XML output.

To add a test: create a `.kvp` file in `test/<dir>/kvp/<RequestType>/` or `.xml` in `test/<dir>/xml/<RequestType>/`, and place the expected response in `test/<dir>/output/<RequestType>/` with the corresponding `.kvp.get` or `.xml.post` suffix.

Ignore lists (`ignore-sqlite`, `ignore-circle-ci`, etc.) skip tests that don't apply to certain backends.
