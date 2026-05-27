project('vconvert', 'c', version: '0.1.0', default_options: ['warning_level=2'])

gtk_dep = dependency('gtk+-3.0')
gio_dep = dependency('gio-2.0')

gnome = import('gnome')
resources = gnome.compile_resources(
    'vconvert-resources',
    'src/vconvert.gresource.xml',
    source_dir: 'src'
)

src_files = [
    'src/main.c',
    'src/domain/media_types.c',
    'src/usecases/converter.c',
    'src/presentation/window.c',
    'src/infrastructure/image_converter.c',
    'src/infrastructure/video_converter.c',
    'src/infrastructure/audio_converter.c',
    resources
]

executable('vconvert',
    src_files,
    dependencies: [gtk_dep, gio_dep],
    install: true
)
