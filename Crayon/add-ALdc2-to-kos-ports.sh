#!/bin/bash

mkdir -p /opt/toolchains/dc/kos-ports/ALdc/files
echo $'\n\
TARGET = libALdc.a\n\
OBJS = AL/mojoal.o AL/aldc.o AL/alut.o\n\
\n\
KOS_CFLAGS += -ffast-math -O3 -Iinclude -Wall -Werror\n\
\n\
include ${KOS_PORTS}/scripts/lib.mk\n\
' > /opt/toolchains/dc/kos-ports/ALdc/files/KOSMakefile.mk

echo $'\n\
PORTNAME = ALdc\n\
PORTVERSION = 1.0.0\n\
\n\
MAINTAINER = Luke Benstead <kazade@gmail.com>\n\
LICENSE = MIT License\n\
SHORT_DESC = OpenAL 1.1 implementation for KOS\n\
\n\
DEPENDENCIES = \n\
\n\
GIT_REPOSITORY =    https://gitlab.com/simulant/aldc.git\n\
\n\
TARGET =			libALdc.a\n\
INSTALLED_HDRS =	include/AL/al.h include/AL/alc.h include/AL/alut.h\n\
HDR_COMDIR =		AL\n\
\n\
KOS_DISTFILES =		KOSMakefile.mk\n\
\n\
include ${KOS_PORTS}/scripts/kos-ports.mk\n\
' > /opt/toolchains/dc/kos-ports/ALdc/Makefile

source /etc/bash.bashrc; cd /opt/toolchains/dc/kos-ports/ALdc && make install clean

#https://gitlab.com/simulant/simulant/blob/master/platforms/dreamcast/Dockerfile#L62
