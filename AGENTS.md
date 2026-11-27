# TriageProject

## Build / verification
- C++17 + Qt6 + Designar. Qt6 REQUIRED components: `Widgets Sql Network Positioning`; OPTIONAL: `Location QuickWidgets` (both found define `HAVE_QT_LOCATION`). Non-Android also needs SQLite3/OpenSSL. Build: `cmake -S . -B build && cmake --build build -j2`.
- Three CMake targets: `TriageProject` (GUI), `triage_api` (REST server, non-Android only), `test_motores` (Qt-free engine tests). No CTest/lint targets (`enable_testing`/`add_test` absent); run binaries directly: `./build/test_motores` (13 checks) and `g++ -std=c++17 -I/usr/local/include tests/main_pruebas.cpp -o /tmp/prueba_designar && /tmp/prueba_designar`.
- Local toolchain is Qt 6.2.4; `Qt6Location` is not installed, so the optional Location/QuickWidgets branch compiles out and `resources/mapa_ubicacion.qml` is not loaded by C++ here.

## Local run / data
- `triage_api` binds `0.0.0.0:8081`; the GUI defaults to `http://127.0.0.1:8081` via `TRIAGE_API_URL` (`src/ApiClient.cpp:16`). Android needs a reachable host-IP override.
- DB path is **cwd-based**: `cwd/data/triage.db`; `resolverRaizProyecto()` walks up from cwd to find `scripts/`. Override with `TRIAGE_DB_PATH` (exact file) or `TRIAGE_DATA_DIR` (dir). `temp_patients.json` always sits next to the DB. On startup the server creates the schema from `scripts/database_setup.sql` if `centers` is missing, migrates missing `centers` columns, and seeds via `scripts/seed.sql` when `centers` is empty. Seed logins: `iahula_admin/admin123`, `sorjuana_admin/123456` (unsalted SHA-256 hex, `scripts/seed.sql:36`).
- Both `data/triage.db` and `data/temp_patients.json` are tracked. POST/admit rewrites `temp_patients.json`; restore demo data with `git checkout -- data/temp_patients.json`.

## Architecture / gotchas
- `src/main.cpp` -> `MainWindow` (QStackedWidget: VistaMain, patient form, login, dashboard). `ApiClient` is the only HTTP layer.
- `BosqueTriaje` supplies the UI category list and symptom options (`categorias()`, `opcionesCategoria(id)`, `nombreCategoria(id)`); `MotorTriaje` consumes it for scoring. Keep these in sync with `FormularioPacienteWidget` calls.
- Triage scoring, GPS and geocoding are client-side. Catalogs load from `:/resources/data/*.json`, so every resource must be listed in `recursos.qrc` and edits require a rebuild. The server trusts submitted `triage_level`/`triage_score`. The form submits `latitude`/`longitude`; the API stores the posted JSON as-is in `temp_patients.json` (the `patients` table has no coordinate columns).
- "Registrar Centro" in `LoginWidget` only inserts into an in-memory Designar AVL; it does not create a DB/API user. Real login is `POST /api/login` against SQLite.
- `build_android/` is untracked (not gitignored) and contains absolute `/home/arianna/...` Qt/NDK paths: do not hand-edit or treat as portable config.
- UI strings and code comments are in Spanish; match that convention.
