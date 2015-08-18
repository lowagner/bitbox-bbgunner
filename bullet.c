#include "bullet.h"
#include <stdlib.h> // rand

void init_guns()
{
    // free everything, and setup the links in the free indices
    for (int p=0; p<2; ++p)
    {
        gun[p].num_bullets_out = 0;
        gun[p].just_fired = 0;
        
        gun[p].first_active_index = -1;
        gun[p].first_free_index = 0;

        gun[p].range = 64;
        gun[p].damage = 16;
        
        for (int i=0; i<5; ++i)
            gun[p].bullet[i] = (Bullet) {
                .next_active_index = -1,
                .next_free_index = i+1
            };

        gun[p].bullet[4].next_free_index = -1;
    }
}

int update_bullet(int16_t p, uint8_t b)
{   
    // return 1 if bullet should be killed off
    Bullet *bb = &gun[p].bullet[b];
    if (!bb->range)
        return 1;
    else
        --bb->range;

    edge *eb = &e[8*p+3+b]; // bullet edge

//  // VERY SIMPLE WAY TO DO THINGS -- doesn't check bullets passing through something too fast

    if (eb->p1.y > 0.0f && eb->p2.y > 0.0f) // the bullet hit the ground last time
        return 1;

    for (int i=0; i<3; ++i)
    {
        eb->p1.world[i] += bb->velocity[i];
        eb->p2.world[i] += bb->velocity[i];
    }

//  // more complicated way of calculating things
//    float dr[3];
//    for (int i=0; i<3; ++i)
//        dr[i] = eb->p2.world[i] - eb->p1.world[i];
//
//    if (dot(dr, bb->velocity) > 0)
//    {   // p2 is in front, p1 is in back
//        //message("\ngot bullet %d:\n front x,y,z = %f, %f, %f\n  back x,y,z = %f, %f, %f\n", b, eb->p2.x, eb->p2.y, eb->p2.z, eb->p1.x, eb->p1.y, eb->p1.z);
//        if (eb->p1.y > 0.0f) // the bullet hit the ground last time
//            return 1;
//
//        // move p1 to front
//        for (int i=0; i<3; ++i)
//            eb->p1.world[i] = eb->p2.world[i] + bb->velocity[i];
//    }
//    else
//    {   // p1 is in front, p2 is in back
//        //message("\ngot bullet %d:\n front x,y,z = %f, %f, %f\n  back x,y,z = %f, %f, %f\n", b, eb->p1.x, eb->p1.y, eb->p1.z, eb->p2.x, eb->p2.y, eb->p2.z);
//        if (eb->p2.y > 0.0f) // the bullet hit the ground last time
//            return 1;
//
//        // move p2 to the front
//        for (int i=0; i<3; ++i)
//            eb->p2.world[i] = eb->p1.world[i] + bb->velocity[i];
//    }


    // check collisions with player (1-p)
    int hit_player = check_collision(eb, 1-p);    
    if (hit_player && player[1-p].health)
    {
        if (player[1-p].health > gun[p].damage)
            player[1-p].health -= gun[p].damage;
        else
            kill_player(1-p);
    }
    return hit_player;
}

inline int CCW(float *p1, float *p2, float *p3)
{
    // return if p1, p2, p3 are oriented CCW  in the x-z plane, looking down on it (in positive y direction)
    //      p3
    //  p1       -> CCW = 1
    //      p2

    return (p3[2] - p1[2]) * (p2[0] - p1[0]) > (p3[0] - p1[0]) * (p2[2] - p1[2]);
}

inline int intersection(edge *e1, edge *e2)
{
    return (CCW(e1->p1.world, e2->p1.world, e2->p2.world) != CCW(e1->p2.world, e2->p1.world, e2->p2.world))  &&
           (CCW(e1->p1.world, e1->p2.world, e2->p1.world) != CCW(e1->p1.world, e1->p2.world, e2->p2.world));
}

int check_collision(edge *eb, int p)
{
    
    float bymin; 
    float bymax;
    if (eb->p1.y > eb->p2.y)
    {
        bymin = eb->p2.y;
        bymax = eb->p1.y;
    }
    else
    {
        bymin = eb->p1.y;
        bymax = eb->p2.y;
    }

    float py = player[p].y;
    if (bymax < py-0.01 || bymin > py + 0.3)
        // too far above or below the player
        return 0;
    if (intersection(eb, &e[8*p]))
        return 1;
    if (intersection(eb, &e[8*p+1]))
        return 1;
    return 0;
}

uint8_t free_bullet(int16_t p, uint8_t b_prev, uint8_t b)
{
    // free the bullet at index b and return the next bullet
    // do this by stitching up the linked list properly.

    // check if b is at the front of the list:
    uint8_t next_b = gun[p].bullet[b].next_active_index;
    if (b == gun[p].first_active_index)
        // fix the front of the list...
        gun[p].first_active_index = next_b;
    else
        // stitch the previous' next to the current's next:
        gun[p].bullet[b_prev].next_active_index = next_b;

    // put bullet b at the front of the free indices, push everything up:
    gun[p].bullet[b].next_free_index = gun[p].first_free_index;
    gun[p].first_free_index = b;

    // remove it from the number of bullets out...
    --gun[p].num_bullets_out;

    // push the edges to infinity
    e[8*p+3+b] = (edge) {
      .p1 = (vertex) { 
        .world = { 10000, 10000, 10000 } },
      .p2 = (vertex) { 
        .world = { 10000, 10000, 10000 } },
      .color = 0
    };

    return next_b;
}

void shoot_bullet(int p)
{
if (!gun[p].just_fired)
{   // you haven't just fired a shot
    if (gun[p].num_bullets_out < 5)
    {
        // you probably can fire!
        if (gun[p].num_bullets_out != 2)
        {
            switch (gun[p].num_bullets_out)
            {
            case 0:
                if (gun[p].range < 254)
                    gun[p].range += 2;
                else
                    gun[p].range = 255;
                break;
            case 1:
                if (gun[p].range < 255)
                    ++gun[p].range;
                break;
            // two does nothing to the range.  fire a third bullet and it
            // won't affect your range/damage.
            case 3:
                if (gun[p].range > 16)
                    --gun[p].range;
                break;
            case 4:
                if (gun[p].range > 17)
                    gun[p].range -= 2;
                else
                    gun[p].range = 16;
                break;
            }
            gun[p].damage = damage_from_range(gun[p].range); 
            message("range to %d, damage %d\n", (int) gun[p].range, (int) gun[p].damage);
        }

        ++gun[p].num_bullets_out;
   
        uint8_t b = gun[p].first_free_index;
        Bullet *bb = &gun[p].bullet[b];

        // update the active bullet list
        bb->next_active_index = gun[p].first_active_index;
        gun[p].first_active_index = b;
        // update the free bullet list
        gun[p].first_free_index = bb->next_free_index;

        bb->damage = gun[p].damage;
        bb->range = gun[p].range;

        float vmultiplier = 0.5*pow(1.0f*gun[p].range, 0.25)/4;
        for (int i=0; i<3; ++i)
            bb->velocity[i] = player[p].velocity[i] + vmultiplier * player[p].facing[i];

        if (gun[p].range < 64)
        {
            // add in some randomness
            vmultiplier = 0.00001*64/gun[p].range;
            bb->velocity[0] += vmultiplier * (rand() % 2048 - 1024);
            bb->velocity[2] += vmultiplier * (rand() % 2048 - 1024);
        }

        e[8*p+3+b] = (edge) {
          .p1 = (vertex) { 
            .world = {player[p].x, player[p].y, player[p].z},
          },
          .p2 = (vertex) { 
            .world = { player[p].x + 0.8*player[p].fx, player[p].y + 0.8*player[p].fy, player[p].z + 0.8* player[p].fz } 
            //.world = { player[p].x + bb->vx, player[p].y + bb->vy, player[p].z + bb->vz } 
          }
        };

        if (p)
          e[8*p+3+b].color = RGB(0,0x99,0xff);
        else
          e[8*p+3+b].color = RGB(0xff,0x99,0);
        
        //message("shooting bullet %d, ", (int) b);
    }
    else
    {
        // all the bullets are out, decrease range and accuracy, also damage.
        if (gun[p].range > 16)
        {
            --gun[p].range;
            gun[p].damage = damage_from_range(gun[p].range); 
        }
        message("range down to %d, damage %d\n", (int) gun[p].range, (int) gun[p].damage);
    }
    gun[p].just_fired = gun[p].range/4 + 2;
}
}
