mkdir _conan
cd _conan
conan install ..
set PKG_CONFIG_PATH=%cd%
cd ..
meson _build
pause
