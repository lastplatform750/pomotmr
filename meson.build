# top-level meson.build
project('myproject', 'c',
        version : '0.1.0',
        meson_version : '>=1.9.1',
        default_options : ['warning_level=2', 'c_std=c23'])

subdir('src')

if get_option('tests')
  subdir('tests')
endif

install_subdir('resource', 
                install_dir: join_paths(get_option('datadir'), 
                'pomotmr'))