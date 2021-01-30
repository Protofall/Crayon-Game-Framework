# Needed so we can use scons stuff like builders
from SCons.Script import *

def get_supported_systems():
	return ['dreamcast', 'all']	# TODO: Add 'pc'

# Used to make the user programs
def get_supported_bootmodes():
	systems = get_supported_systems()
	bootmodes = list()
	for s in systems:
		if s == 'dreamcast':
			bootmodes.append(s + '-cd')
			bootmodes.append(s + '-pc')
			bootmodes.append(s + '-sd')
		elif s == 'all':
			bootmodes.append('all')
		else:
			print('System: ' + s + ' bootmodes unknown!')
			exit(1)
	return bootmodes

# Used to make the libraries
def get_supported_libraries():
	systems = get_supported_systems()
	libraries = list()
	for s in systems:
		if s == 'dreamcast':
			libraries.append(s)
			libraries.append(s + '-fat32')
			libraries.append(s + '-zlib')
			libraries.append(s + '-fat32-zlib')
		elif s == 'all':
			libraries.append('all')
		else:
			print('System: ' + s + ' libraries unknown!')
			exit(1)
	return libraries

# This will prevent 'none' from working
	# Will also prevent `scons --help` from working
	# TBH, choosing build `none` isn't terrible...
def valid_build(key, val, env):
	# If you try BUILDS=none, this function enters with an empty val...
	if val == "":
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
			default = 0),
	)

	# TODO: Fix this since its really broken (BUILDS=invalid)
	# SUPER BUGGY
	# This is the only way to set a validator for ListVariable. LHS is tuple
		# TODO. Currently this only works for all-caps 'BUILDS'
		# Also help doesn't work unless a platform is given
	(key, help, default, _, converter) = ListVariable('BUILDS',
		help = "The specific library builds we want to create",
		default = 'unspecified',
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

	Help(vars.GenerateHelpText({}))	# Needs a parameter, but it doesn't have to look at it

	# vars.FormatVariableHelpText() might be useful

	arguments = dict()
	arguments['DEBUG'] = processing_env['DEBUG']

	# Split the BUILDS into a list
	target_builds = str(processing_env['BUILDS']).split(',')

	# If 'all' was present, just set it to all platforms
	if 'all' in target_builds:
		target_builds = get_supported_libraries()
		target_builds.remove('all')

	arguments['BUILDS'] = target_builds

	return arguments

# "params" is the command line arguments, "our_vars" is just a dict with "CRAYON_BASE" and such
def create_builders(params, our_vars):
	import os
	env = list()

	# GCC colour output was only added in this version
	colour_version = [4, 9, 0]

	from sys import platform
	for b in params['BUILDS']:
		if b.startswith('dreamcast'):
			env.append(
				Environment(
					ENV = os.environ,
					CC = 'kos-cc',
					CXX = 'kos-c++',
					AR = 'kos-ar',
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
				env[-1].AppendUnique(LIBS = 'lkosfat')	# not needed?
			if 'zlib' in b:
				env[-1].AppendUnique(CPPDEFINES = {'ZLIB':1})
				env[-1].AppendUnique(LIBS = 'lz')

		elif b.startswith('pc'):
			# Apparently some ppl need os' ENV for CCVERSION
			env.append(Environment(ENV = os.environ))

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

		env[-1]['CODE_DIR'] = 'code'
		# env[-1]['CDFS_DIR'] = 'cdfs'
		# if 'PROG_NAME' in our_vars:
		# 	env[-1]['PROG_NAME'] = our_vars['PROG_NAME']

		#Add in some cflags if in debug mode
		env[-1]['DEBUG'] = params['DEBUG']
		if env[-1]['DEBUG'] == True:
			# Wformat level 2 has extra checks over standard.
			# no-common is where two files define the same global var when they should be seperate
			# g3 is like g, but it includes macro information
			env[-1].AppendUnique(CPPFLAGS = ['-g3', '-Wall', '-Wformat=2', '-fno-common'])
		else:
			env[-1].AppendUnique(CPPFLAGS = '-Wno-unused-parameter')

		# Shadow will tell us about vars in multiple scopes clashing and extra is good
		# no-unused-parameter disables the warning for unused parameters
		env[-1].AppendUnique(CPPFLAGS = ['-Wshadow', '-Wextra', '-Wno-unused-parameter'])

		# Enables GCC colour (Since it normally only does colour for terminals and scons is just an "output")
		# Major, Minor, Patch version numbers
		# We need the CC and CXX checks for pc because this flag is only for GCC/G++
		our_version = list(map(int, env[-1]['CCVERSION'].split('.')))
		if all([a >= b for a, b in zip(our_version, colour_version)]) and (env[-1]['GENERAL_PLATFORM'] != 'pc' or (env[-1]['CC'] == 'gcc' or env[-1]['CXX'] == 'g++')):
			env[-1].AppendUnique(CCFLAGS = ['-fdiagnostics-color=always'])

	return env
