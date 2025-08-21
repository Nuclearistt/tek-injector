project(
  'tek-injector',
  'cpp',
  version: files('meson.version'), meson_version: '>=1.4.0',
  license: 'GPL-3.0-or-later', license_files: 'COPYING',
  default_options: {
    'cpp_std': 'gnu++23',
    'warning_level': '3'
  }
)
add_project_arguments(
  '-gcodeview', '-DUNICODE','-D_UNICODE', '-DTEK_INJ_IMPL',
  language: 'cpp'
)
add_project_link_arguments('-municode', '-static', language: 'cpp')
# Make file version for the .rc file
rc_version = meson.project_version().replace('.', ',')
if rc_version.contains('-')
  rc_version = rc_version.split('-')[0]
endif
num_elems = rc_version.split(',').length()
if num_elems == 2
  rc_version += ',0,0'
endif
if num_elems == 3
  rc_version += ',0'
endif
compiler = meson.get_compiler('cpp')
# -Wno-attributes: Project code uses many attributes that are supported only by
#    specific compilers or even specific compiler versions
# -Wno-nullability-extension: Fallbacks for these attributes are provided by
#    tek-injector.h anyway
add_project_arguments(
  compiler.get_supported_arguments(
    '-Wno-attributes', '-Wno-nullability-extension'),
  language: 'cpp'
)
install_headers('include/tek-injector.h')
winmod = import('windows')
libtek_injector = library(
  'tek-injector',
  'src/lib.cpp',
  winmod.compile_resources(
    configure_file(
      input: 'res/libtek-injector.rc.in',
      output: 'libtek-injector.rc',
      configuration: {
        'file_version': rc_version,
        'pretty_version': meson.project_version()
      }
    )
  ),
  cpp_args: '-DTEK_INJ_EXPORT',
  cpp_static_args: '-DTEK_INJ_STATIC',
  link_args: '-Wl,--pdb=libtek-injector.pdb',
  gnu_symbol_visibility: 'hidden',
  include_directories: 'include',
  install: true
)
static_arg_arr = (
  get_option('default_library') == 'static'
  or (
    get_option('default_library') == 'both'
    and meson.version().version_compare('>=1.6.0')
    and get_option('default_both_libraries') == 'static'
  )
) ? '-DTEK_INJ_STATIC' : []
import('pkgconfig').generate(
  libtek_injector,
  description: 'DLL injector for tek-game-runtime',
  extra_cflags: static_arg_arr,
  url: 'https://github.com/teknology-hub/tek-injector'
)
configure_file(
  input: 'res/tek-injector.manifest.in',
  output: 'tek-injector.manifest',
  configuration: {'version': rc_version.replace(',', '.')}
)
executable(
  'tek-injector',
  'src/exe.cpp',
  winmod.compile_resources(
    configure_file(
      input: 'res/tek-injector.rc.in',
      output: 'tek-injector.rc',
      configuration: {
        'file_version': rc_version,
        'pretty_version': meson.project_version()
      }
    )
  ),
  cpp_args: static_arg_arr,
  link_args: '-Wl,--pdb=tek-injector.pdb',
  gnu_symbol_visibility: 'hidden',
  include_directories: 'include',
  install: true,
  link_with: libtek_injector,
  win_subsystem: 'windows'
)
