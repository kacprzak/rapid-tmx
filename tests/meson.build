boost_dep = dependency('boost',
                       modules: ['unit_test_framework'],
                       required: false,
                       disabler: true)

test_exe = executable('map_test', 'map_test.cpp',
                      dependencies: [boost_dep, rapid_tmx_dep])

test('map test', test_exe,
     workdir: meson.current_source_dir())
