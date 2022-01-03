project('rapid-tmx', 'cpp',
        version: '0.1.1')

inc = include_directories('include', 'external')

zlib_dep = dependency('zlib')
boost_dep = dependency('boost')

base64_src = files('external/base64/base64.cpp')

subdir('include')
subdir('src')

rapid_tmx_dep = declare_dependency(
  include_directories: include_directories('include'),
  link_with: lib,
  dependencies: [zlib_dep, boost_dep]
)

subdir('tests')
