project(
	'shaderbg',
	'c',
	version: '0.0',
	meson_version: '>=0.48.0',
	default_options: [
		'c_std=c11',
	],
)


add_project_arguments(
	[
		'-D_DEFAULT_SOURCE',
	],
	language: 'c',
)

wayland_client = dependency('wayland-client', version: '>=1.20')
wayland_egl = dependency('wayland-egl')
egl = dependency('egl')
GL = dependency('GL')


wayland_scanner = find_program('wayland-scanner')

wayland_scanner_code = generator(
	wayland_scanner,
	output: '@BASENAME@-protocol.c',
	arguments: ['private-code', '@INPUT@', '@OUTPUT@'],
)

wayland_scanner_client = generator(
	wayland_scanner,
	output: '@BASENAME@-client-protocol.h',
	arguments: ['client-header', '@INPUT@', '@OUTPUT@'],
)

client_protos_src = []
client_protos_headers = []

client_protocols = [
	['wlr-layer-shell-unstable-v1.xml'],
	['xdg-shell.xml'],
]

foreach p : client_protocols
	xml = join_paths(p)
	client_protos_src += wayland_scanner_code.process(xml)
	client_protos_headers += wayland_scanner_client.process(xml)
endforeach

deps = [wayland_client, GL, wayland_egl, egl]

shaderbg = executable(
	'shaderbg',
	['main.c'] + client_protos_src + client_protos_headers,
	dependencies: deps,
	install : true
)
