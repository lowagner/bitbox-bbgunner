# DO NOT FORGET to define BITBOX environment variable 



NAME = bbgunner
GAME_C_FILES = wview3d.c player.c bullet.c bb3d.c main.c song.c chiptune.c
GAME_H_FILES = wview3d.h player.h bullet.h bb3d.h common.h song.h

#ENGINE_FILES += chiptune.c

GAME_C_OPTS += -DVGA_MODE=640 #-DDEBUG


# see this file for options
include $(BITBOX)/kernel/bitbox.mk

