# Needed so we can use scons stuff like builders
from SCons.Script import *

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
def valid_build(key, val, env):
	# If you try BUILDS=none, this function enters with an empty value
	# But we don't want to do this when in help mode
	if val == "" and GetOption('help') != True:
		print("Please give a value for BUILDS. Type \"scons --help\" for more information")
		Exit(1)

	# Split val so we can check all arguments
	for v in val.split():
		if not v in get_supported_libraries():
			print("Please give a value for BUILDS. Type \"scons --help\" for more information")
			Exit(1)

def input_logic(args):
	vars = Variables('scons_args.py', args)
	vars.AddVariables(
		BoolVariable('DEBUG',
			help = "Build in debug mode",
			default = 0
		),
	)

	# TODO: Fix this since its really broken (BUILDS=invalid)
	# SUPER BUGGY
	# This is the only way to set a validator for ListVariable. LHS is tuple
		# TODO. Currently this only works for all-caps 'BUILDS'
		# Also help doesn't work unless a platform is given
	(key, help, default, _, converter) = ListVariable('BUILDS',
		help = "The specific library builds we want to create",
		default = 'none',
		names = get_supported_libraries()
	)	# I might be able to set map to map "dc" to dreamcast...
	vars.Add((key, help, default, valid_build, converter))

	# Have to do this to access params
	processing_env = Environment(tools = [], variables = vars)

	# Must be done after env creation...
	unknown_params = vars.UnknownVariables()
	if unknown_params:
		print("Invalid options detected: " + ', '.join(map(str, unknown_params.keys())))
		Exit(1)

	help_message = vars.GenerateHelpText({})	# Needs a parameter, but it doesn't have to look at it
	Help(help_message)

	# Since we're doing weird stuff with the ListVariable, this is how I get it to print the message
	if GetOption('help') == True:
		print(help_message)
		Exit(1)

	return vars

# "params" is the command-line/scons_arg.py arguments
# "our_vars" is just a dict with "CRAYON_BASE" and such
def create_builders(params, our_vars):
	import os
	env = list()

	# This is the only way to check the variables in "params"
	args_env = Environment(tools = [], variables = params)

	# Get all the libs in a way that we can iterate over them
	target_builds = str(args_env['BUILDS']).split(',')
	print(target_builds)
	if 'all' in target_builds:
		target_builds = get_library_names(get_supported_systems())

	# GCC colour output was only added in this version
	colour_version = [4, 9, 0]

	from sys import platform
	for b in target_builds:
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
			print(b)

		env[-1]['SPECIFIC_BUILD'] = b

		# Ensure CRAYON_BASE is set
		if 'CRAYON_BASE' in our_vars:
			env[-1]['CRAYON_BASE'] = our_vars['CRAYON_BASE']
			env[-1].AppendUnique(CPPPATH = ['$CRAYON_BASE/include/'])
		else:
			print('CRAYON_BASE is missing, please add the path')
			Exit(1)

		# # Stuff for building the project
		# env[-1]['CODE_DIR'] = 'code'
		# env[-1]['CDFS_DIR'] = 'cdfs'
		# if 'PROG_NAME' in our_vars:
		# 	env[-1]['PROG_NAME'] = our_vars['PROG_NAME']

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
			# Disabled the unused parameters/variables warnings that can be a little annoying
			env[-1].AppendUnique(CPPFLAGS = ['-Wno-unused-parameter', '-Wno-unused-value'])

		# Shadow will tell us about vars in multiple scopes clashing and extra is good
		# no-unused-parameter disables the warning for unused parameters
		env[-1].AppendUnique(CPPFLAGS = ['-Wshadow', '-Wextra', '-Wno-unused-parameter'])

		# Enables GCC colour (Since it normally only does colour for terminals and scons is just an "output")
		# Major, Minor, Patch version numbers
		# We need the CC and CXX checks for pc because this flag is only for GCC/G++
			# TODO: Add a check to make sure compiler is a GCC variant. This is useless for clang and stuff
		our_version = list(map(int, env[-1]['CCVERSION'].split('.')))
		if all([a >= b for a, b in zip(our_version, colour_version)]) and (env[-1]['GENERAL_PLATFORM'] != 'pc' or (env[-1]['CC'] == 'gcc' or env[-1]['CXX'] == 'g++')):
			env[-1].AppendUnique(CCFLAGS = ['-fdiagnostics-color=always'])

	return env
