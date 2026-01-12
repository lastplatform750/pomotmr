tests_enabled = get_option('tests')

if tests_enabled
  inc = include_directories('..' / 'include')

  test_exe = executable('test_example',
    'test_main.c',
    include_directories: inc)

  test('unit_example', test_exe)
endif