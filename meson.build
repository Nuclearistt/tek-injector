project(
  'TEK Injector',
  'c',
  version: '1.0.3',
  default_options: {
    'optimization': '2',
    'warning_level': '3',
    'b_vscrt': 'mt',
    'c_std': 'c23',
    'c_winlibs': [
      'advapi32.lib',
      'kernel32.lib',
      'libucrt.lib',
      'libvcruntime.lib',
      'shell32.lib',
      'user32.lib',
    ],
  },
)
executable(
  'tek-injector',
  'main.c',
  'vm-stubs.s',
  import('windows').compile_resources('tek-injector.rc'),
  win_subsystem: 'windows',
  c_args: '/GS-',
  link_args: [
    '/ENTRY:entry',
    '/MANIFEST:EMBED',
    '/MANIFESTINPUT:' + meson.current_source_dir() / 'tek-injector.manifest',
    '/MANIFESTUAC:level=\'requireAdministrator\'',
  ],
)
