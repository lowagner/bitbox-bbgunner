#ifndef COMMON_H
#define COMMON_H

#include "bitbox.h"
#include "bb3d.h"
#include <stdint.h> // uint

#define MAX_EDGES 32
#define GRAVITY 0.03f

typedef struct _player {
    union {
        float world[3];
        struct {
            float x, y, z;
        };
    };
    union {
        float facing[3];
        struct {
            float fx, fy, fz;
        };
    };
    union {
        float velocity[3];
        struct {
            float vx, vy, vz;
        };
    };
    uint8_t health;
    int8_t dhealthdt; // change in health over time
    uint8_t ammo;
    int8_t dammodt;  // change in ammo over time
    uint8_t speed; // player speed
    int8_t movement; // forward/backward movement
    int8_t strafe;
    int8_t omega; // angular velocity, or something like it
    uint8_t jump_impulse;

    uint8_t other; 
    int8_t other2;
    uint8_t other3; 
} Player;

typedef struct _bullet {
    uint8_t next_active_index; // index of the next active bullet
    uint8_t next_free_index; //
    uint8_t damage, range;
    union {
        float velocity[3];
        struct {
            float vx, vy, vz;
        };
    };
} Bullet;

typedef struct _gun {
    uint8_t num_bullets_out; // how many bullets are flying
    uint8_t just_fired;  // count-down til next bullet can be fired
    uint8_t info1, info2;
    uint8_t first_active_index, first_free_index, damage, range;
    Bullet bullet[5];
} Gun;

extern Camera camera[2];
extern Player player[2];
extern Gun gun[2];

extern int nume;
extern edge e[MAX_EDGES]; // array of edges

#ifdef EMULATOR
extern int debug_draw;
#endif

inline void kill_player(int p)
{
    player[p].health = 0;
    player[p].dhealthdt = -128; // this will roughly correspond with respawn time
}

inline uint8_t damage_from_range(uint8_t range)
{
    return range/4 + (range*range/512);
}


#endif
