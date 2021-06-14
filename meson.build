project('bg',['cpp'],version: '0.1')

configure_file(input: 'lbpcascade_frontalface_improved.xml',
  output: 'lbpcascade_frontalface_improved.xml',
  copy: true)
configure_file(input: 'options.txt',
  output: 'options.txt',
  copy: true)
executable(meson.project_name(),'bg.cpp',
dependencies: [dependency('opencv4')],install: false)
