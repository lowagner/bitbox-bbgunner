#ifndef BULLET_H
#define BULLET_H

#include "common.h"

void init_guns();

void shoot_bullet(int p);
int update_bullet(int16_t p, uint8_t b);
uint8_t free_bullet(int16_t p, uint8_t b_prev, uint8_t b); // free bullet index b and return next bullet index

int check_collision(edge *eb, int p);

inline void update_bullets()
{
    for (int p=0; p<2; ++p)
    {
        uint8_t prev_b = -1;
        uint8_t b = gun[p].first_active_index;
        while (b < 255)
        {
            if (update_bullet(p, b))
                b = free_bullet(p, prev_b, b);
            else
            {
                prev_b = b;
                b = gun[p].bullet[b].next_active_index;
            }
        }
    }
}

#endif
