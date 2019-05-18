from SCons.Script import *	# Needed so we can use scons stuff like builders

#UNFINISHED...probably
def Init(env):

	#Set env vars (How will I do this because this depends on var from the caller)
	# PROJECT	= $(CRAYON_PROJ_NAME)
	# CFLAGS	= $(CRAYON_PROJ_CFLAGS) $(KOS_CFLAGS) -I$(CRAYON_BASE)/include
	# LDFLAGS	= $(CRAYON_PROJ_LDFLAGS) $(KOS_LDFLAGS) -L$(CRAYON_BASE)/lib/dreamcast
	# LIBS	= $(CRAYON_PROJ_LIBS) -lcrayon -lm $(KOS_LIBS)
	# IP_BIN	= $(CRAYON_IP_BIN)$(if $(CRAYON_IP_BIN),,$(CRAYON_BASE)/IP.BIN)
	# BUILD	= $(CRAYON_BUILD_DIR)

	# print env['CRAYON_BASE']	#It is present

	#Make builders
	elf = Builder(action="kos-cc -o $TARGET $SOURCES $LIBS")	#SOURCES takes all dependencies and shoves them into one command
	kos_bin = Builder(action="sh-elf-objcopy -R .stack -O binary $SOURCE $TARGET")
	scramble = Builder(action="$KOS_BASE/utils/scramble/scramble $SOURCE $TARGET")
	iso = Builder(action="mkisofs -G $KOS_BASE/../IP.BIN -C 0,11702 -J -l -r -o $TARGET .")
	cdi = Builder(action="cdi4dc $SOURCE $TARGET")

	#Add the builders
	env.Append(BUILDERS= {'Elf': elf, 'KosBin': kos_bin, 'Scramble': scramble, 'Iso': iso, 'Cdi': cdi})
	env.Append(CRAYON_TEST= "true m8")
