project('rapid-tmx', 'cpp',
        version: '0.1.1')

inc = include_directories('include', 'external')

zlib_dep = dependency('zlib')

rapidxml_dep = dependency('rapidxml', required: false)
if not rapidxml_dep.found()
   cpp = meson.get_compiler('cpp')

   cpp.has_header('rapidxml/rapidxml.hpp', required: true)
   cpp.has_header('rapidxml/rapidxml_utils.hpp', required: true)
endif

base64_src = files('external/base64/base64.cpp')

subdir('include')
subdir('src')

rapid_tmx_dep = declare_dependency(
  include_directories: include_directories('include'),
  link_with: lib
)

subdir('tests')

pkg = import('pkgconfig')
pkg.generate(lib,
             description: 'Tiled level editor tmx files support.')

if build_machine.system() != 'windows'
   ln = find_program('ln')

   run_command(ln, '-sf',
               meson.project_build_root() / 'compile_commands.json',
               meson.project_source_root())
endif
