project('reader_library', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++17', 'werror=true'])
add_project_arguments('-pedantic', language : 'cpp')

gtest_proj = subproject('gtest')
gtest_dep = gtest_proj.get_variable('gtest_main_dep')
gmock_dep = gtest_proj.get_variable('gmock_dep')

efs_test = executable('test_efs',
  'test/test_crc16.cpp',
  'test/test_integration.cpp',
  'test/test_parser.cpp',
  'test/test_result.cpp',
  dependencies : [gtest_dep, gmock_dep],
  include_directories : include_directories('components/efs/'))

test('efs tests', efs_test, protocol: 'gtest')
