#include <stdlib.h> // rand
#include "bb3d.h"
#include "wview3d.h"
#include "player.h"
#include "bullet.h"
#include "song.h"
#include "chiptune.h"

/*
TODO:  
rare bug on bitbox:  screen will go black for a very specific configuration.
  possible causes:
    * too many edges on screen needing special calculations 
     (i.e. too many going from front of view to back of view) -- I think this is it!
    * division by zero somewhere -- unlikely??
*/

edge e[MAX_EDGES]; // array of edges
int nume; // number of vertices
#ifdef EMULATOR
int debug_draw;
#endif

Camera camera[2];
Player player[2];
Gun gun[2];

float speed, rotspeed; // helpful for setting player velocities

inline void move_player_pieces(int p)
{
    // when dead, do something
    // use player[0].x,y,z for velocity of edge 1
    // use player[0].fx, fy, fz for velocity of edge 2
}

inline void init_player_attributes(int p)
{
    player[p].omega = 0;
    player[p].speed = 80;
    player[p].jump_impulse = 10;
    player[p].movement = 0;
    player[p].strafe = 0;
    player[p].ammo = 255;
    player[p].health = 255;
    player[p].dtdhealth = 0;

    // reset gun attributes
    gun[p].range = 64;
    gun[p].damage = damage_from_range(gun[p].range); 
    player[p].dtdammo = dtdammo_from_range(gun[p].range); // how much time passes before we add an ammo
}

inline void respawn_player(int p)
{
    if (player[1-p].health)
    {
        if (rand()%2)
            player[p] = (Player) {
                .world = {
                    player[1-p].x - 10*player[1-p].fx,
                    0.0f,
                    player[1-p].z - 10*player[1-p].fz
                },
                .facing = {
                    camera[1-p].right[0],
                    camera[1-p].right[1],
                    camera[1-p].right[2]
                },
                .velocity = {0.0f, 0.0f, 0.0f}
            };
        else
            player[p] = (Player) {
                .world = {
                    player[1-p].x - 10*player[1-p].fx,
                    0.0f,
                    player[1-p].z - 10*player[1-p].fz
                },
                .facing = {
                    -camera[1-p].right[0],
                    -camera[1-p].right[1],
                    -camera[1-p].right[2]
                },
                .velocity = {0.0f, 0.0f, 0.0f}
            };
    }
    else
    {
        player[p] = (Player) {
            .facing = { 1.0f-2.0f*p, 0.0f, 0.0f },
            .world = { 5.0f-10.0f*p, 0.0f, 0.0f },
            .velocity = {0.0f, 0.0f, 0.0f}
        };
    }

    init_player_attributes(p);
    
    reset_player_view(p); // reset his/her edges
}


void game_init()
{
    // setup the game with some random vertices
    nume = 17; // 3 for each player, 5 for bullets for each player, plus one joiner
    // the third edge of the character is not visible until death.
    
    // bullets
    init_guns();

    // make players stand "back to back":
    player[0] = (Player) { 
        .world = {5.0, 0.0, 0.0},
        .facing = {1.0, 0.0, 0.0}
    };
    init_player_attributes(0);
    reset_player_view(0);
    player[1] = (Player) {
        .world = {-5.0, 0.0, 0.0},
        .facing = {-1.0, 0.0, 0.0}
    };
    init_player_attributes(1);
    reset_player_view(1);

    message("putting bullets at \"infinity\"\n");
    for (int p=0; p<2; ++p)
    for (int i=0; i<6; ++i)
    {
        e[8*p+2+i] = (edge) { 
          .p1 = (vertex) { 
            .world = { 10000, 10000, 10000 } },
          .p2 = (vertex) { 
            .world = { 10000, 10000, 10000 } },
          .color = 0
        };
    }
    // last edge joins the players so you know where to shoot!
    e[16] = (edge) {
        .p1 = (vertex) {
            .world = {player[0].x, player[0].y, player[0].z},
        },
        .p2 = (vertex) {
            .world = {player[1].x, player[1].y, player[1].z},
        },
        .color = RGB(0xff,0,0)
    };

    // get the vertices' screen positions:
    get_all_coordinates();
    init_drawing_edges();


    // the graphing algorithm (for displaying on screen) benefits from the edges
    // being sorted top to bottom, here we do it with heap sort, O(nume lg nume):
    heap_sort_edges(0);
    #ifdef DEBUG
    message("player 0 view:\n");
    for (int i=0; i<nume; ++i)
    {
        message("e[%02d].p1.world=(%f, %f, %f), .image=(%d, %d, %f)\n", i, e[i].p1.world[0], e[i].p1.world[1], e[i].p1.world[2], e[i].p1.image[0], e[i].p1.image[1], e[i].p1.image_z);
        message("     .p2.world=(%f, %f, %f), .image=(%d, %d, %f)\n", e[i].p2.world[0], e[i].p2.world[1], e[i].p2.world[2], e[i].p2.image[0], e[i].p2.image[1], e[i].p2.image_z);
    }
    #endif
    heap_sort_edges(1);
    #ifdef DEBUG
    message("\nplayer 1 view:\n");
    for (int i=0; i<nume; ++i)
    {
        message("e[%02d].p1.world=(%f, %f, %f), .image=(%d, %d, %f)\n", i, e[i].p1.world[0], e[i].p1.world[1], e[i].p1.world[2], e[i].p1.image[0], e[i].p1.image[1], e[i].p1.image_z);
        message("     .p2.world=(%f, %f, %f), .image=(%d, %d, %f)\n", e[i].p2.world[0], e[i].p2.world[1], e[i].p2.world[2], e[i].p2.image[0], e[i].p2.image[1], e[i].p2.image_z);
    }
    #endif
    chip_play(&what_chipsong);
}

void game_frame()
{
    if (chip_song_finished())
        chip_play(&what_chipsong);

    kbd_emulate_gamepad();

    // move camera to arrow keys (or d-pad):
    for (int p=0; p<2; ++p)
    {
        if (player[p].health)
        {

            // run player physics:
            speed = player[p].speed * 0.001953125;
            rotspeed = player[p].speed * 0.00044828125;
            if (player[p].y == 0.0f) // player on ground
                move_player_ground(p);
            else // player is in the air
                move_player_air(p);

            // player can shoot or get hit by bullets on ground or in the air:
            if (GAMEPAD_PRESSED(p, B))
                shoot_bullet(p);
            else if (player[p].ammo < 255)
            {
                if (vga_frame % player[p].dtdammo == 0)
                    ++player[p].ammo;
            }
            
            if (update_player(p)) // check if player dies by poisoning!
            {   
                kill_player(p);
            }
        } // end player[p].health != 0
        else // player's health is zero!
        {
            if (player[p].dtdhealth < 127)
            {
                ++player[p].dtdhealth;
                // make player asplode
                move_player_pieces(p);
            }
            else
            {
                // give player new life somewhere
                respawn_player(p);
            }
        }
    }
    // setup danger line! -- joins both players
    if (player[0].health && player[1].health)
    {
        e[16].p1.x = player[0].x + 0.5*player[0].fx;
        e[16].p1.y = player[0].y + 0.1;
        e[16].p1.z = player[0].z + 0.5*player[0].fz;
        e[16].p2.x = player[1].x + 0.5*player[1].fx;
        e[16].p2.y = player[1].y + 0.1;
        e[16].p2.z = player[1].z + 0.5*player[1].fz;
        //e[16].color = RGB(0xff,0,0);
    }
    else
    {
        e[16].p1.x = 10000;
        e[16].p1.y = 10000;
        e[16].p1.z = 10000;
        e[16].p2.x = 10000;
        e[16].p2.y = 10000;
        e[16].p2.z = 10000;
    }
    // update all the bullets
    update_bullets();
}
