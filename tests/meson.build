tests_enabled = get_option('tests')

if tests_enabled
  alsa_dep = dependency('alsa', required : true)

  inc = include_directories('..' / 'include')

  ring_test_exe = executable('ring_test',
    '../src/ring.c',
    'ring_test.c',
    dependencies: [alsa_dep],
    include_directories: inc)

  test('ring_test', ring_test_exe, timeout: 60)
endif