project(
	'wlsunset',
	'c',
	'cpp',
	version: '0.2.0',
	license: 'MIT',
	meson_version: '>=0.56.0',
	default_options: [
		'c_std=c11',
		'warning_level=3',
		'werror=true',
	],
)

add_project_arguments(
	[
		'-Wundef',
		'-Wunused',
		'-Wlogical-op',
		'-Wmissing-include-dirs',
		'-Wold-style-definition', # nop
		'-Wpointer-arith',
		'-Wstrict-prototypes',
		'-Wmissing-prototypes',
		'-Wno-implicit-fallthrough',
		'-Wno-unknown-warning-option',
		'-Wno-unused-command-line-argument',
		'-Wvla',
		'-Wl,--exclude-libs=ALL',
		'-DWLSUNSET_VERSION="@0@"'.format(meson.project_version()),
	],
	language: 'c',
)

add_project_arguments(
	[
		'-std=c++17',
		'-Wundef',
		'-Wunused',
		'-Wlogical-op',
		'-Wmissing-include-dirs',
		'-Wpointer-arith',
		'-Wno-implicit-fallthrough',
		'-Wvla',
		'-Wl,--exclude-libs=ALL',
		'-D_USE_MATH_DEFINES',
		'-D_XOPEN_SOURCE=700',
		'-DWLSUNSET_VERSION="@0@"'.format(meson.project_version()),
	],
	language: 'cpp',
)

scanner = find_program('wayland-scanner')
scanner_private_code = generator(scanner, output: '@BASENAME@-protocol.c', arguments: ['private-code', '@INPUT@', '@OUTPUT@'])
scanner_client_header = generator(scanner, output: '@BASENAME@-client-protocol.h', arguments: ['client-header', '@INPUT@', '@OUTPUT@'])

protocols_src = [scanner_private_code.process('wlr-gamma-control-unstable-v1.xml', 'xdg-output-unstable-v1.xml')]
protocols_headers = [scanner_client_header.process('wlr-gamma-control-unstable-v1.xml', 'xdg-output-unstable-v1.xml')]

wl_client = dependency('wayland-client')
wl_protocols = dependency('wayland-protocols')
lib_protocols = static_library('protocols', protocols_src + protocols_headers, dependencies: wl_client)
protocols_dep = declare_dependency(link_with: lib_protocols, sources: protocols_headers)

cc = meson.get_compiler('c')
m = cc.find_library('m')
rt = cc.find_library('rt')

server_glue = custom_target(
	'wlsunset-server-glue.h',
	output: 'wlsunset-server-glue.h',
	input: 'wlsunset.xml',
	command: ['sdbus-c++-xml2cpp', '@INPUT@', '--adaptor=@OUTPUT@'],
)

client_glue = custom_target(
	'wlsunset-client-glue.h',
	output: 'wlsunset-client-glue.h',
	input: 'wlsunset.xml',
	command: ['sdbus-c++-xml2cpp', '@INPUT@', '--proxy=@OUTPUT@'],
)

executable(
	'wlsunset',
	['main.c', 'color_math.c', 'str_vec.c', 'dbus.cc', server_glue],
	dependencies: [wl_client, protocols_dep, m, rt, dependency('sdbus-c++')],
	install: true,
)

executable(
	'wlsunsetctl',
	['wlsunsetctl.cc', client_glue],
	dependencies: [dependency('sdbus-c++'), dependency('absl_any_invocable'), dependency('absl_str_format'), dependency('absl_strings')],
	install: true,
)

install_data(
	'wlsunset.xml',
	install_dir: 'share/dbus-1/interfaces',
)

install_data(
	'wlsunset.service',
	install_dir: 'share/dbus-1/services',
)

scdoc = dependency('scdoc', required: get_option('man-pages'), version: '>= 1.9.7', native: true)

if scdoc.found()
	scdoc_prog = find_program(scdoc.get_variable(pkgconfig: 'scdoc'), native: true)
	mandir = get_option('mandir')

	foreach src : ['wlsunset.1.scd']
		topic = src.split('.')[0]
		section = src.split('.')[1]
		output = '@0@.@1@'.format(topic, section)

		custom_target(
			output,
			input: src,
			output: output,
			command: [
				'sh', '-c', '@0@ < @INPUT@ > @1@'.format(scdoc_prog.full_path(), output)
			],
			install: true,
			install_dir: '@0@/man@1@'.format(mandir, section)
		)
	endforeach
endif
