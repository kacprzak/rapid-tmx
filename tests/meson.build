boost_dep = dependency('boost', modules: ['unit_test_framework'])

test_exe = executable('map_test', 'map_test.cpp',
                      include_directories: inc,
                      link_with: lib,
                      dependencies: [boost_dep, rapid_tmx_dep])

test('map test', test_exe,
     workdir: meson.current_source_dir())
