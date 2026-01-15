project_src = 'src'

inc = include_directories('..' / 'include')

alsa_dep = dependency('alsa', required : true)
curses_dep = dependency('ncurses', required : true)


# an executable that links the library
exe = executable('pomotmr',
                 'cl_args.c',
                 'ring.c',
                 'timer.c',
                 'interface.c',
                 'main.c',
                 include_directories: inc,
                 dependencies: [alsa_dep, curses_dep],
                 install: true)
