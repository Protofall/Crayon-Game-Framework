# Needed so we can use scons stuff like builders
from SCons.Script import *

def get_env_types_from_bootmodes(bootmodes, env):
	result = list()

	for b in bootmodes:
		if b.startswith('dreamcast'):
			name = 'dreamcast'
			if b == 'dreamcast-sd' or env['FAT32'] == True:
				name = '-'.join([name, 'fat32'])
			if env['ZLIB'] == True:
				name = '-'.join([name, 'zlib'])
			if name not in result:
				result.append(name)
		else:
			print('ERROR: Unknown boot mode detected in \"get_env_types_from_bootmodes()\"')
			Exit(1)

	return result

def get_supported_systems():
	return ['dreamcast']	# TODO: Add 'pc'

def get_bootmode_names(systems):
	bootmodes = list()
	for s in systems:
		if s == 'dreamcast':
			bootmodes.append(s + '-cd')
			bootmodes.append(s + '-pc')
			bootmodes.append(s + '-sd')
		else:
			print('System: ' + s + ' bootmodes unknown!')
			return list()

	return bootmodes

def get_library_names(systems, get_all = False):
	libraries = list()
	for s in systems:
		if s == 'dreamcast':
			libraries.append(s)
			libraries.append(s + '-fat32')
			libraries.append(s + '-zlib')
			libraries.append(s + '-fat32-zlib')
		else:
			print('System: ' + s + ' libraries unknown!')
			return list()

	return libraries

def get_supported_bootmodes():
	return get_bootmode_names(get_supported_systems())

def get_supported_libraries():
	return get_library_names(get_supported_systems())

# This will prevent 'none' from working
	# Will also catch `help`, but I force it to ignore it and I handle it later
def crayon_validation(val, mode):
	# If you try BUILDS=none, this function enters with an empty value
	# But we don't want to do this when in help mode
	if val == '' and GetOption('help') != True:
		print('Please give a value for BUILDS. Type \"scons --help\" for more information')
		Exit(1)

	builds_list = []
	if mode == 'library':
		builds_list = get_supported_libraries()
	elif mode == 'project':
		builds_list = get_supported_bootmodes()
	else:
		print('ERROR: Invalid \"mode\" for \"crayon_validation()\"')
		Exit(1)

	# Split val so we can check all arguments
	for v in val.split():
		if v not in builds_list:
			print('Please give a valid value for BUILDS. Type \"scons --help\" for more information')
			Exit(1)
	

def valid_library_build(key, val, env):
	crayon_validation(val, 'library')

def valid_project_build(key, val, env):
	crayon_validation(val, 'project')

def input_logic(args, mode):
	help_text = ''
	modeInt = 0
	if mode != 'library' and mode != 'project':
		print('ERROR: Invalid \"mode\" for \"input_logic()\"')
		Exit(1)

	vars = Variables('scons_args.py', args)

	vars.AddVariables(
		BoolVariable('DEBUG',
			help = 'Build in debug mode',
			default = 0
		),
	)

	# For libs, this is defined by the build instead
		# However projects that build with bootmode dreamcast-sd will always use FAT32
	if mode == 'project':
		import os
		crayon_base = os.environ['CRAYON_BASE']

		vars.AddVariables(
			BoolVariable('FAT32',
				help = 'Compiles FAT32 support into your program. Be aware of licenses!',
				default = 0
			),
			BoolVariable('ZLIB',
				help = 'Compiles ZLIB support into your program. Be aware of licenses!',
				default = 0
			),
			PathVariable('IP_BIN',
				help = 'The initial program needed for Dreamcast games',
				default = crayon_base + '/IP.BIN',
				validator = PathVariable.PathIsFile
			),
			('PROJECT_NAME', 'The name of your project', os.path.basename(os.getcwd())),
		)
	elif mode == 'library':
		vars.Add(PathVariable('CRAYON_BASE',
			help = 'Location of libCrayon',
			default = './',
			validator = PathVariable.PathIsDir
		))

	# TODO: Fix this since its really broken (BUILDS=invalid)
	# SUPER BUGGY
	# This is the only way to set a validator for ListVariable. LHS is tuple
		# TODO. Currently this only works for all-caps 'BUILDS'
		# Also help doesn't work unless a platform is given
	(key, help, default, _, converter) = ListVariable('BUILDS',
		help = 'The specific ' + ('library' if mode == 'library' else 'bootmode') + ' builds we want to create',
		default = 'none',
		names = (get_supported_libraries() if mode == 'library' else get_supported_bootmodes())
	)	# I might be able to set map to map 'dc' to dreamcast...
	vars.Add((key, help, default, (valid_library_build if mode == 'library' else valid_project_build), converter))

	# Have to do this to access params
	processing_env = Environment(tools = [], variables = vars)

	# Must be done after env creation...
	unknown_params = vars.UnknownVariables()
	if unknown_params:
		print('Invalid options detected: ' + ', '.join(map(str, unknown_params.keys())))
		Exit(1)

	help_message = vars.GenerateHelpText({})	# Needs a parameter, but it doesn't have to look at it
	Help(help_message)

	# Since we're doing weird stuff with the ListVariable, this is how I get it to print the message without continuing
	if GetOption('help') == True:
		print(help_message)
		Exit(1)

	return vars

def build_project(args):
	params = input_logic(args, 'project')

	envs = create_builders(params, 'project')

	if len(envs) < 1:
		print('ERROR: Somehow we didn\'t create any envs')
		Exit(1)

	lib_folder = envs[0]['CRAYON_BASE']
	lib_dict = SConscript(lib_folder + '/SConscript', exports = 'envs')

	return

# 'params' is the command-line/scons_arg.py arguments
def create_builders(params, mode):
	import os

	# This is the only way to check the variables in 'params'
	args_env = Environment(tools = [], variables = params)

	# Get all the libs in a way that we can iterate over them
	target_builds = str(args_env['BUILDS']).split(',')

	if mode == 'library':
		if 'all' in target_builds:
			target_builds = get_library_names(get_supported_systems())
		env_names = target_builds
	elif mode == 'project':
		if 'all' in target_builds:
			target_builds = get_bootmode_names(get_supported_systems())
		env_names = get_env_types_from_bootmodes(target_builds, args_env)
	else:
		print('ERROR: Invalid \"mode\" for \"create_builders()\"')
		Exit(1)

	from sys import platform
	env = list()
	for b in env_names:
		if b.startswith('dreamcast'):
			env.append(
				Environment(
					ENV = os.environ,
					CC = 'kos-cc',
					CXX = 'kos-c++',
					AR = 'kos-ar',
					variables = params,
				)
			)

			# Making sure we use the right prefix and suffix
			env[-1]['LIBPREFIX'] = 'lib'
			env[-1]['LIBSUFFIX'] = '.a'
			env[-1]['OBJSUFFIX'] = '.o'	# Windows has .obj
			# env[-1]['PROGSUFFIX'] = '.elf'

			# Fix this later, here's a hack for now
			env[-1]['KOS_BASE'] = env[-1]['ENV']['KOS_BASE']
			# env[-1]['KOS_GENROMFS'] = env[-1]['ENV']['KOS_GENROMFS']

			# # Location of IP.BIN
			# if 'IP_BIN_DIR' in our_vars:
			# 	env[-1]['IP_BIN_DIR'] = our_vars['IP_BIN_DIR'] + 'IP.BIN'

			# Add the platform
			env[-1]['GENERAL_PLATFORM'] = 'dreamcast'
			env[-1]['SPECIFIC_PLATFORM'] = env[-1]['GENERAL_PLATFORM']

			# # TODO: Not requied just yet
			# env[-1].AppendUnique(LIBS = ['-ALdc'])

			# Parts of these might not be necessary
			if 'fat32' in b:
				env[-1].AppendUnique(CPPDEFINES = {'FAT32':1})
				env[-1].AppendUnique(LIBS = 'lkosfat')	# not needed for making the lib?
			if 'zlib' in b:
				env[-1].AppendUnique(CPPDEFINES = {'ZLIB':1})
				env[-1].AppendUnique(LIBS = 'lz')

		elif b.startswith('pc'):
			# Apparently some ppl need os' ENV for CCVERSION
			env.append(
				Environment(
					ENV = os.environ,
					variables = params
				)
			)

			# Add the platform
			env[-1]['GENERAL_PLATFORM'] = 'pc'
			if platform.startswith('linux') == True:
				env[-1]['SPECIFIC_PLATFORM'] = 'linux'
			elif platform == 'win32':
				env[-1]['SPECIFIC_PLATFORM'] = 'windows'
			elif platform == 'darwin':
				env[-1]['SPECIFIC_PLATFORM'] = 'mac'
			else:
				print('Computer platform "' + platform + '" is not supported')
				Exit(1)
		else:	# Should never get here
			print('Somehow got here: ' + b)

		# Set the specific lib and boot modes we'll want to use
		env[-1]['SPECIFIC_BUILD'] = b
		# env[-1]['SPECIFIC_BOOTMODE'] = 
		env[-1]['SPECIFIC_LIBRARY'] = b

		# Set the CRAYON_BASE related stuff
		if 'CRAYON_BASE' not in env[-1]:
			if 'CRAYON_BASE' not in os.environ:
				print('CRAYON_BASE is missing from PATHs and args, please add it')
				Exit(1)
			env[-1]['CRAYON_BASE'] = os.environ['CRAYON_BASE']

		env[-1].AppendUnique(CPPPATH = ['$CRAYON_BASE/include/'])

		# # Stuff for building the project
		# env[-1]['CODE_DIR'] = 'code'
		# env[-1]['BUILD_DIR'] = 'build'
		# env[-1]['CDFS_DIR'] = 'cdfs'
		# env[-1]['PROGRAM_DIR'] = 'program'

		# We convert debug to an int so we get `-DCRAYON_DEBUG=0/1` instead of true/false
		env[-1].AppendUnique(CPPDEFINES = {'CRAYON_DEBUG': int(env[-1]['DEBUG'])})

		#Add in some cflags if in debug mode
		if env[-1]['DEBUG'] == True:
			# conversion will check for type conversions (eg uint8_t var = (U32 VAR))
			# g3 is like g, but it includes macro information
			# Wformat level 2 has extra checks over standard.
			# no-common is where two files define the same global var when they should be seperate
			env[-1].AppendUnique(CPPFLAGS = ['-Wconversion', '-g3', '-Wall', '-Wformat=2', '-fno-common'])
		else:
			# Disabled the unused parameters/variables/labels warnings that can be a little annoying
			env[-1].AppendUnique(CPPFLAGS = ['-Wno-unused-parameter', '-Wno-unused-value', '-Wno-unused-label'])

		# Shadow will tell us about vars in multiple scopes clashing and extra is good
		env[-1].AppendUnique(CPPFLAGS = ['-Wshadow', '-Wextra'])

		# Enables GCC colour (Since it normally only does colour for terminals and scons is just an 'output')
		# Major, Minor, Patch version numbers (This version was the one that added the flag)
		gcc_colour_version = [4, 9, 0]
		# We need the CC and CXX checks for pc because this flag is only for GCC/G++
			# TODO: Add a check to make sure compiler is a GCC variant. This is useless for clang and stuff
		our_version = list(map(int, env[-1]['CCVERSION'].split('.')))
		if all([a >= b for a, b in zip(our_version, gcc_colour_version)]) and (env[-1]['GENERAL_PLATFORM'] != 'pc' or (env[-1]['CC'] == 'gcc' or env[-1]['CXX'] == 'g++')):
			env[-1].AppendUnique(CCFLAGS = ['-fdiagnostics-color=always'])

	return env
