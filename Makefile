# DO NOT FORGET to define BITBOX environment variable 

USE_3D = 1

NAME = bbgunner
GAME_C_FILES = wview3d.c player.c bullet.c main.c
GAME_H_FILES = wview3d.h player.h bullet.h common.h

GAME_C_OPTS += -DVGAMODE_640 #-DDEBUG

# see this file for options
include $(BITBOX)/lib/bitbox.mk

