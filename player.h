#ifndef PLAYER_H
#define PLAYER_H

#define TRIPOD_HEIGHT 1.2f // distance of camera above player
#define TRIPOD_FOLLOW 1.3f // distance back from player to camera

extern float speed, rotspeed; // helpful for setting player velocities

void reset_player_view(int p);
void move_player_ground(int p);
void move_player_air(int p);
int update_player(int p); // update player attributes and check bullet collisions

#endif
