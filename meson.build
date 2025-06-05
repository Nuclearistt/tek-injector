project(
  'tek-injector',
  'c', 'cpp',
  version: '1.2.1',
  default_options: {
    'warning_level': '3',
    'c_std': 'c23',
    'cpp_std': 'c++23'
  }
)
executable(
  'tek-injector',
  'main.c',
  'memscan.cpp',
  'vm-stubs.s',
  import('windows').compile_resources('tek-injector.rc'),
  link_args: '-static',
  win_subsystem: 'windows'
)
