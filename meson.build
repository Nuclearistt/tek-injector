project(
  'TEK Injector',
  'c',
  version: '1.0.2',
  default_options: {
    'optimization': '3',
    'warning_level': '3',
    'b_lto': 'true',
    'b_lto_mode': 'thin',
    'b_vscrt': 'mt',
    'c_std': 'c2x',
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
res = import('windows').compile_resources(
  'tek-injector.rc',
  include_directories: '/usr/x86_64-pc-windows-msvc/include',
)
executable(
  'tek-injector',
  'main.c',
  'vm-stubs.s',
  res,
  win_subsystem: 'windows',
  c_args: '/GS-',
  link_args: [
    '/ENTRY:entry',
    '/MANIFEST:EMBED',
    '/MANIFESTINPUT:../tek-injector.manifest',
    '/MANIFESTUAC:level=\'requireAdministrator\'',
  ],
)
