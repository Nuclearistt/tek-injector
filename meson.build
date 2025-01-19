project(
  'tek-injector',
  'c', 'cpp',
  version: '1.1.0',
  default_options: {
    'warning_level': '3',
    'c_std': 'c23',
    'cpp_std': 'c++latest'
  }
)
executable(
  'tek-injector',
  'main.c',
  'vm-stubs.s',
  import('windows').compile_resources('tek-injector.rc'),
  win_subsystem: 'windows',
  c_args: '/GS-',
  cpp_args: '/GS-',
  link_args: [
    '/ENTRY:entry',
    '/MANIFEST:EMBED',
    '/MANIFESTINPUT:' + meson.current_source_dir() / 'tek-injector.manifest',
    '/MANIFESTUAC:level=\'requireAdministrator\''
  ]
)
