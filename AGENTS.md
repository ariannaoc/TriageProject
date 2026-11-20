# TriageProject

## Build / verification
- CMake C++17 Qt6 app: requires Qt6 `Widgets Sql Network`, Designar (`/usr/local/lib/cmake/Designar`), SQLite3 and OpenSSL. AUTOMOC/AUTORCC/AUTOUIC are on.
- Build with `cmake -S . -B build && cmake --build build -j2`. Targets: `build/TriageProject` (GUI) and `build/triage_api` (REST server). `build/` and `build_android/` are generated, gitignored; `build_android/` contains absolute `/home/arianna/...` Qt/NDK paths — do not hand-edit or treat as portable config.
- `tests/` is not wired into CMake (no CTest/lint/CI targets). Run its Designar smoke test manually: `g++ -std=c++17 -I/usr/local/include tests/main_pruebas.cpp -o /tmp/prueba_designar && /tmp/prueba_designar` (`main_pruebas.cpp` includes `prueba_designar.cpp`).

## Local run / data
- `data/triage.db` is gitignored, but `triage_api` creates it at startup from `scripts/database_setup.sql` + `seed.sql` when tables are missing or `centers` is empty. Manual setup also works: `sqlite3 data/triage.db < scripts/database_setup.sql && sqlite3 data/triage.db < scripts/seed.sql`. Seed logins: `iahula_admin/admin123`, `sorjuana_admin/123456`; passwords are unsalted SHA-256 hex (`scripts/seed.sql:28`).
- DB and `data/temp_patients.json` paths resolve from the executable location (walks up from `build/`/`build_android/` to the repo root), so the server works from either the repo root or `build/`; override with `TRIAGE_DB_PATH` (exact `.db` file) or `TRIAGE_DATA_DIR` (directory). Startup prints the resolved path. It rewrites the tracked `data/temp_patients.json` on POST/admit; restore demo data with `git checkout -- data/temp_patients.json` when done.
- `svr.listen` at `api_server/server.cpp` binds `0.0.0.0:8081`, so both the GUI default `127.0.0.1:8081` and LAN/Android clients can connect; keep these in sync if either side changes.
- The GUI's `TRIAGE_API_URL` defaults to `http://127.0.0.1:8081` (`src/ApiClient.cpp:15`). Android builds must override it with a reachable host IP, and the server must bind a matching interface.

## Architecture / gotchas
- `src/main.cpp` -> `MainWindow` (QStackedWidget of `VistaMain.ui`, `FormularioPacienteWidget`, `LoginWidget`, dashboard). `ApiClient` is the only HTTP layer; the dashboard, form and login communicate with it via signals.
- "Registrar Centro" in `LoginWidget` only inserts into an in-memory Designar AVL; it does not create a DB/API user and cannot be used to log in. Real login is `POST /api/login` against SQLite.
- Triage scoring is client-side in `FormularioPacienteWidget` from `resources/data/{symptoms,triage_levels,risk_factors}.json` loaded via `:/resources/...`; the server trusts the submitted `triage_level`/`triage_score`. Editing those catalogs requires a rebuild.
- The API is a single `api_server/server.cpp` using vendored `include/libs/httplib.h` and `include/libs/json.hpp`, not system packages.
- Resources must be listed in `recursos.qrc` to exist under `:/resources/...`; all UI icons and the three triage JSON catalogs are embedded there, so the GUI reads no local project files.
- UI strings and code comments are in Spanish; match that convention.
