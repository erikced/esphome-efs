project('reader_library', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++17', 'werror=true'])
add_project_arguments('-pedantic', language : 'cpp')

gtest = subproject('gtest')
gtest_dep = dependency('gtest', main : true)

efs_test = executable('test_efs',
  'test/main.cpp',
  'test/test_reader.cpp',
  'test/test_crc16.cpp',
  'test/test_parser.cpp',
  'test/test_integration.cpp',
  dependencies : gtest_dep,
  include_directories : include_directories('components/efs/'))

test('efs tests', efs_test)
