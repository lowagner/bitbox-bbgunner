#include "player.h"
#include "common.h"

const uint16_t gamepad_jump = gamepad_A + gamepad_select;
const uint16_t gamepad_slow = gamepad_Y + gamepad_start;

const float a = 0.28867513459; //sqrt(3)/6;
void reset_player_view(int p)
{
    // setup the camera
    camera[p] = (Camera) {
        .viewer = { player[p].x - TRIPOD_FOLLOW*player[p].fx, 
                    player[p].y - TRIPOD_HEIGHT*(1 - 0.07*gun[p].range/64), // *(0.9+6.4/gun[p].range), // on purpose to make camera follow up/down motions less
                    player[p].z - TRIPOD_FOLLOW*player[p].fz},
        .viewee = { player[p].x + 2*(0.6+0.4*gun[p].range/64)*player[p].fx, 
                    player[p].y, // + 1*player[p].fy,  // see above!
                    player[p].z + 2*(0.6+0.4*gun[p].range/64)*player[p].fz},
        .down = {0,1,0},
        .magnification = 300
    };
    get_view(&camera[p]);
    #ifdef DEBUG
    message("cam matrix: ");
    for (int i=0; i<12; ++i)
        message("%f, ", camera[p].view_matrix[i]);
    message("\n");
    #endif

    e[8*p] = (edge) {
        .p1 = (vertex) { 
            .world = { player[p].x+2*a*player[p].fx,
                       player[p].y+2*a*player[p].fy,
                       player[p].z+2*a*player[p].fz } 
        },
        .p2 = (vertex) { 
            .world = { player[p].x-a*player[p].fx+0.5*camera[p].right[0],
                       player[p].y-a*player[p].fy+0.5*camera[p].right[1],
                       player[p].z-a*player[p].fz+0.5*camera[p].right[2] } 
        },
        .color = RGB(0xff, 0xff, 0xff)
    };
    e[8*p+1] = (edge) {
        .p1 = (vertex) { 
            .world = { player[p].x+2*a*player[p].fx,
                       player[p].y+2*a*player[p].fy,
                       player[p].z+2*a*player[p].fz } 
        },
        .p2 = (vertex) { 
            .world = { player[p].x-a*player[p].fx-0.5*camera[p].right[0],
                       player[p].y-a*player[p].fy-0.5*camera[p].right[1],
                       player[p].z-a*player[p].fz-0.5*camera[p].right[2] } 
        },
        .color = RGB(0xff, 0xff, 0xff)
    };

}

void move_player_ground(int p)
{
    if (GAMEPAD_PRESSED(p, slow))
    {
        speed /= 2;
        rotspeed /= 4;
    }

    for (int i=0; i<3; i++)
        player[p].velocity[i] = 0;

    int need_new_view = 0;
    if (GAMEPAD_PRESSED(p, L)) 
    {
        if (player[p].strafe > 0)
            player[p].strafe = 0;
        else
            player[p].strafe = -64;
    }
    else if (GAMEPAD_PRESSED(p, R)) 
    {
        if (player[p].strafe < 0)
            player[p].strafe = 0;
        else
            player[p].strafe = 64;
    }
    if (GAMEPAD_PRESSED(p, left)) 
    {
        if (player[p].omega > 0)
            player[p].omega = 0;
        else
            player[p].omega = -64;
    }
    else if (GAMEPAD_PRESSED(p, right)) 
    {
        if (player[p].omega < 0)
            player[p].omega = 0;
        else
            player[p].omega = 64;
    }
    if (GAMEPAD_PRESSED(p, down))
    {
        if (player[p].movement > 0)
            player[p].movement = 0;
        else
            player[p].movement = -64;
    }
    else if (GAMEPAD_PRESSED(p, up))
    {
        if (player[p].movement < 0)
            player[p].movement = 0;
        else
            player[p].movement = 64;
    }
    if (GAMEPAD_PRESSED(p, jump))
    {
        player[p].vy = -1.0*player[p].jump_impulse/64;
        need_new_view = 1;
    }
    if (player[p].movement)
    {
        for (int i=0; i<3; ++i)
            player[p].velocity[i] += speed*player[p].movement*player[p].facing[i]/64;
        player[p].movement /= 2;
        need_new_view = 1;
    }
    if (player[p].omega)
    {
        for (int i=0; i<3; ++i)
            player[p].facing[i] += player[p].omega*rotspeed*camera[p].right[i]/64;
        normalize(player[p].facing, player[p].facing);
        player[p].omega /= 2;
        need_new_view = 1;
    }
    if (player[p].strafe)
    {
        for (int i=0; i<3; ++i)
            player[p].velocity[i] += speed*player[p].strafe*camera[p].right[i]/72;
        player[p].strafe /= 2;
        need_new_view = 1;
    }
    if (need_new_view)
    {
        for (int i=0; i<3; ++i)
            player[p].world[i] += player[p].velocity[i];
        reset_player_view(p);
    }
    #ifdef EMULATOR
    if (GAMEPAD_PRESSED(p, start))
    {
        if (!debug_draw)
        {
            debug_draw = 2;
            for (int i=0; i<nume; ++i)
            {
                message("e[%02d].p1.world=(%f, %f, %f), .image=(%d, %d, %f)\n", i, e[i].p1.world[0], e[i].p1.world[1], e[i].p1.world[2], e[i].p1.image[0], e[i].p1.image[1], e[i].p1.image_z);
                message("     .p2.world=(%f, %f, %f), .image=(%d, %d, %f)\n", e[i].p2.world[0], e[i].p2.world[1], e[i].p2.world[2], e[i].p2.image[0], e[i].p2.image[1], e[i].p2.image_z);
            }
            message("\n");
        }
    }
    #endif
}

void move_player_air(int p)
{
    player[p].x += player[p].vx;
    player[p].y += player[p].vy;
    player[p].z += player[p].vz;
    if (player[p].omega)
    {
        //rotate player because s/he has angular velocity
        for (int i=0; i<3; ++i)
            player[p].facing[i] += player[p].omega*rotspeed*camera[p].right[i]/64;
    }
    if (player[p].y >= 0.0f)
    {   // player hit ground
        player[p].y = 0.0f; // keep player at ground level
        player[p].vy = 0.0f; // remove y-velocity
        player[p].fy = 0.0f; // remove y-component of facing direction
        int movement = round(dot(player[p].velocity, player[p].facing)/speed);
        if (movement > 127)
            player[p].movement = 127;
        else if (movement < -127)
            player[p].movement = -127;
        else
            player[p].movement = movement;
    }
    else
    {
        // player is in the air, affect using gravity
        player[p].vy += GRAVITY;
        // also angle player down a bit during flight
        player[p].fy = 1.0/8 - player[p].vy;
    }
    normalize(player[p].facing, player[p].facing);
    reset_player_view(p);
}


int update_player(int p) // update player attributes
{   // return 1 if player dies
    // return 0 if player is ok
    if (gun[p].just_fired > 0)
        --gun[p].just_fired;
    
    return 0;
}
