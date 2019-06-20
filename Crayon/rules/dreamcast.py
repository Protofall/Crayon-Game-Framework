from SCons.Script import *	# Needed so we can use scons stuff like builders

#UNFINISHED...probably
def Init(env):

	if 'CRAYON_BASE' not in env['ENV']:
		print "ERROR, variable " + "" + " not found. Stopping program ..."
		quit()

	if 'CRAYON_PROJ_NAME' not in env['ENV']:
		print "ERROR, variable " + "" + " not found. Stopping program ..."
		quit()

	if 'CRAYON_PROJ_LIBS' not in env['ENV']:
		print "ERROR, variable " + "" + " not found. Stopping program ..."
		quit()

	if 'CRAYON_BUILD_DIR' not in env['ENV']:
		print "ERROR, variable " + "" + " not found. Stopping program ..."
		quit()

	#Set env vars
	PROJECT=env['ENV']['CRAYON_PROJ_NAME']
	CFLAGS=""
	LDFLAGS=""
	LIBS=""
	IP_BIN=""
	BUILD=env['ENV']['CRAYON_BUILD_DIR']

	#Setting CFLAGS
	if 'CRAYON_PROJ_CFLAGS' in env['ENV']:
		CFLAGS += env['ENV']['CRAYON_PROJ_CFLAGS'] + " "
	if 'KOS_CFLAGS' in env['ENV']:
		CFLAGS += env['ENV']['KOS_CFLAGS'] + " "
	CFLAGS	+= "-I" + env['ENV']['CRAYON_BASE'] + "/include"

	#Setting LDFLAGS
	if 'CRAYON_PROJ_LDFLAGS' in env['ENV']:
		LDFLAGS += env['ENV']['CRAYON_PROJ_LDFLAGS'] + " "
	if 'KOS_LDFLAGS' in env['ENV']:
		LDFLAGS += env['ENV']['KOS_LDFLAGS'] + " "
	LDFLAGS += "-L" + env['ENV']['CRAYON_BASE'] + "/lib/dreamcast"

	#Setting LIBS
	if 'CRAYON_PROJ_LIBS' in env['ENV']:
		LIBS += env['ENV']['CRAYON_PROJ_LIBS'] + " "
	LIBS += "-lcrayon -lm"
	if 'KOS_LIBS' in env['ENV']:
		LIBS += " " + env['ENV']['KOS_LIBS']

	#Setting IP.BIN location
	if 'CRAYON_IP_BIN' in env['ENV']:
		IP_BIN = env['ENV']['CRAYON_IP_BIN']
	else:
		IP_BIN = env['ENV']['CRAYON_BASE'] + "/IP.BIN"

	#Add our vars
	env['ENV'].update(PROJECT=PROJECT, CFLAGS=CFLAGS,
		LDFLAGS=LDFLAGS, LIBS=LIBS, IP_BIN=IP_BIN, BUILD=BUILD)

	#Make builders
	elf = Builder(action="kos-cc -o $TARGET $SOURCES $LIBS")	#SOURCES takes all dependencies and shoves them into one command
	kos_bin = Builder(action="sh-elf-objcopy -R .stack -O binary $SOURCE $TARGET")
	scramble = Builder(action="$KOS_BASE/utils/scramble/scramble $SOURCE $TARGET")
	iso = Builder(action="mkisofs -G $KOS_BASE/../IP.BIN -C 0,11702 -J -l -r -o $TARGET .")
	cdi = Builder(action="cdi4dc $SOURCE $TARGET")

	#Add the builders
	env.Append(BUILDERS= {'Elf': elf, 'KosBin': kos_bin, 'Scramble': scramble, 'Iso': iso, 'Cdi': cdi})
	
	#Test var
	# env['ENV'].update(CRAYON_TEST = "true m8")
