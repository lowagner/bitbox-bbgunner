// wview3d.h: wireframe view 3d
#ifndef WIREFRAME_VIEW3D_H
#define WIREFRAME_VIEW3D_H
#include "common.h"

#define SCREEN_W 640
#define SCREEN_H 480
#define NEAR_Z 0.1f // things closer than this are invisible, sortof.
#define FAR_Z 1000.0f // things farther than this are invisible

typedef struct hacky_linked_list {
    edge *active_edge;  // "pointer" to edge data
    uint8_t next_active_index; // "pointer" to the next element in the list.
    uint8_t next_free_index; // index which is the next one free.
} Ledger;

Ledger ledger[MAX_EDGES]; // active (currently being drawn) and free edge list
uint8_t first_active_index;
uint8_t first_free_index;
void init_drawing_edges();
//unsigned int effort;

extern edge e[MAX_EDGES]; // array of edges
edge* se[2][MAX_EDGES]; // pointers to sorted edges
extern int nume; // number of edges

void heap_sort_edges(int p); // takes player index p=0 or 1
void get_all_coordinates();

extern Camera camera[2];

#ifdef EMULATOR
int debug_draw;
#endif

inline void get_coordinates(int p, int i)
{
    // this algorithm is a little more complicated in the
    // *edge* cases where one vertex is behind the camera, or both, or none.
    float view1[3], view2[3];
    // calculate the view coordinates (from the pov of the camera):
    matrix_multiply_vector0(view1, camera[p].view_matrix, e[i].p1.world);
    matrix_multiply_vector0(view2, camera[p].view_matrix, e[i].p2.world);
    #ifdef EMULATOR
    int write = 0;
    float view1copy[3], view2copy[3];
    for (int i=0; i<3; ++i)
    {
        view1copy[i] = view1[i];
        view2copy[i] = view2[i];
    }
    #endif
    #ifdef EMULATOR
    if (debug_draw)
        message("edge %d:  ", i);
    #endif
    //effort += 2;
    // now do some testing for the edge-cases when a vertex is behind the camera:
    if (view1[2] > view2[2])
    { 
        // z1 > z2
        if (view1[2] > NEAR_Z && view2[2] < FAR_Z)
        {   // z1 > 0 -- vertex 1 in front of camera
            //effort += 2;
            if (view2[2] > NEAR_Z)
            {   // z2 > 0 -- vertex 2 in front of camera
                // proceed normally:
                // apply perspective to the coordinates:
                view1[0] *= camera[p].magnification / view1[2]; 
                view1[1] *= camera[p].magnification / view1[2]; 
                e[i].p1.ix = SCREEN_W/2 + round(view1[0]);
                e[i].p1.iy = (1+2*p)*SCREEN_H/4 + round(view1[1]);
                
                view2[0] *= camera[p].magnification / view2[2]; 
                view2[1] *= camera[p].magnification / view2[2]; 
                e[i].p2.ix = SCREEN_W/2 + round(view2[0]);
                e[i].p2.iy = (1+2*p)*SCREEN_H/4 + round(view2[1]);
                #ifdef EMULATOR
                if (debug_draw)
                    message("good z\n");
                #endif
            }
            else
            {   // z2 < 0 but z1 > 0
                // these checks could probably be optimized but i'm being lazy now.
                // find the place where the line from p1 to p2, in the camera's frame,
                // intersects the box border on the screen
                //effort += 4;
                float t, a, b, c, diag;
                diag = sqr(401.0)/sqr(camera[p].magnification);
                a = -diag*sqr(view2[2]-view1[2]) + sqr(view2[1]-view1[1]) + sqr(view2[0]-view1[0]);
                b = diag*(view2[2]-view1[2])*view1[2] - (view2[1]-view1[1])*view1[1] - (view2[0]-view1[0])*view1[0];
                c = sqr(view2[0])*(diag*sqr(view1[2]) - sqr(view1[1])) 
                    + diag*sqr(view2[1]*view1[2]-view1[1]*view2[2]) 
                    + 2*view1[0]*view2[0]*(view1[1]*view2[1]-diag*view1[2]*view2[2]) 
                    + sqr(view1[0])*(diag*sqr(view2[2]) - sqr(view2[1]));
                b = b/a;
                c = sqrt(c)/a;
                float original_t = view1[2]/(view1[2]-view2[2]);
                t = original_t;
                #ifdef EMULATOR
                if (debug_draw)
                    message("hello z2 < z1\n", i);
                #endif
                // try to find the smallest t possible, since a small t is closer to view1,
                // and view2 is possibly off screen.
                if (t > 1)
                {
                    // t should default to 1 in case view2 is actually on screen (rare)
                    t = 1;
                    #ifdef EMULATOR
                    if (debug_draw)
                        message("  t = 1\n");
                    #endif
                }
                if (b+c > 0 && b+c < t)
                    t = b+c;
                if (b-c > 0 && b-c < t)
                    t = b-c;
                #ifdef EMULATOR
                if (debug_draw)
                {
                    message("  finally t = %f, from b+c = %f, b-c = %f\n",t, (b+c), (b-c));
                }
                #endif
                if (t == original_t)
                {
                    // the z2 coordinate is at z=0, can't divide here.
                    e[i].p1.iy = 10000;
                    e[i].p2.iy = 10001;
                    #ifdef EMULATOR
                    write = 2;
                    if (debug_draw)
                        message("zero z\n");
                    #endif
                }
                else
                {
                    // now that we have info about the line connecting the two,
                    // find the intersection with the edge of the screen
                    view2[0] = view1[0]*(1-t) + view2[0]*t;
                    view2[1] = view1[1]*(1-t) + view2[1]*t;
                    view2[2] = view1[2]*(1-t) + view2[2]*t;

                    // apply perspective to both coordinates:
                    view1[0] *= camera[p].magnification / view1[2]; 
                    view1[1] *= camera[p].magnification / view1[2]; 
                    e[i].p1.ix = SCREEN_W/2 + round(view1[0]);
                    e[i].p1.iy = (1+2*p)*SCREEN_H/4 + round(view1[1]);
                    
                    view2[0] *= camera[p].magnification / view2[2]; 
                    view2[1] *= camera[p].magnification / view2[2]; 
                    e[i].p2.ix = SCREEN_W/2 + round(view2[0]);
                    e[i].p2.iy = (1+2*p)*SCREEN_H/4 + round(view2[1]);
                    #ifdef EMULATOR
                    write = 2;
                    #endif
                }
            }
        }   // end z2 > 0.0f
        else 
        {   // z1 <= 0, and z2 < z1, or too far forward in z, so put this edge off screen:
            #ifdef EMULATOR
            if (debug_draw)
                message("out of bounds\n");
            #endif
            e[i].p1.iy = 10000;
            e[i].p2.iy = 10001;
        }
    }
    else // z1 <= z2
    {
        // reverse of the above, no comments
        if (view2[2] > NEAR_Z && view1[2] < FAR_Z)
        {   
            //effort += 2;
            if (view1[2] > NEAR_Z)
            {   
                #ifdef EMULATOR
                if (debug_draw)
                    message("good z\n");
                #endif
                view1[0] *= camera[p].magnification / view1[2]; 
                view1[1] *= camera[p].magnification / view1[2]; 
                e[i].p1.ix = SCREEN_W/2 + round(view1[0]);
                e[i].p1.iy = (1+2*p)*SCREEN_H/4 + round(view1[1]);
                
                view2[0] *= camera[p].magnification / view2[2]; 
                view2[1] *= camera[p].magnification / view2[2]; 
                e[i].p2.ix = SCREEN_W/2 + round(view2[0]);
                e[i].p2.iy = (1+2*p)*SCREEN_H/4 + round(view2[1]);
            }
            else // reminder, z1 < z2
            {   
                //effort += 4;
                float t, a, b, c, diag;
                diag = sqr(401.0)/sqr(camera[p].magnification);
                a = -diag*sqr(view2[2]-view1[2]) + sqr(view2[1]-view1[1]) + sqr(view2[0]-view1[0]);
                b = diag*(view2[2]-view1[2])*view1[2] - (view2[1]-view1[1])*view1[1] - (view2[0]-view1[0])*view1[0];
                c = sqr(view2[0])*(diag*sqr(view1[2]) - sqr(view1[1])) + diag*sqr(view2[1]*view1[2]-view1[1]*view2[2]) + 2*view1[0]*view2[0]*(view1[1]*view2[1]-diag*view1[2]*view2[2]) + sqr(view1[0])*(diag*sqr(view2[2]) - sqr(view2[1]));
                b = b/a;
                c = sqrt(c)/a;
                // try to find the largest t possible, since that is the furthest
                // away from view1 which intersects the screen.
                float original_t = view1[2]/(view1[2]-view2[2]);
                t = original_t;
                #ifdef EMULATOR
                if (debug_draw)
                    message("hello z1 < z2\n", i);
                #endif
                if (t < 0)
                {
                    // here t should default to 0 in case view1 is on screen!  rare.
                    t = 0;
                    #ifdef EMULATOR
                    if (debug_draw)
                        message("  t = 0\n");
                    #endif
                }
                if (b+c > t && b+c < 1)
                    t = b+c;
                if (b-c > t && b-c < 1)
                    t = b-c;
                #ifdef EMULATOR
                if (debug_draw)
                {
                    message("  finally t = %f, from b+c = %f, b-c = %f\n",t, (b+c), (b-c));
                    message("  also, original t = %f\n",original_t);
                }
                #endif
                if (t == original_t)
                {
                    // the z1 coordinate is at z=0, can't divide here.
                    e[i].p1.iy = 10000;
                    e[i].p2.iy = 10001;
                    #ifdef EMULATOR
                    write = 4;
                    if (debug_draw)
                        message("zero z\n");
                    #endif
                }
                else
                {
                    // interpolate between the two view points 
                    view1[0] = view1[0]*(1-t) + view2[0]*t;
                    view1[1] = view1[1]*(1-t) + view2[1]*t;
                    view1[2] = view1[2]*(1-t) + view2[2]*t;
                   
                    // then add perspective to both
                    view1[0] *= camera[p].magnification / view1[2]; 
                    view1[1] *= camera[p].magnification / view1[2]; 
                    e[i].p1.ix = SCREEN_W/2 + round(view1[0]);
                    e[i].p1.iy = (1+2*p)*SCREEN_H/4 + round(view1[1]);
                    
                    view2[0] *= camera[p].magnification / view2[2]; 
                    view2[1] *= camera[p].magnification / view2[2]; 
                    e[i].p2.ix = SCREEN_W/2 + round(view2[0]);
                    e[i].p2.iy = (1+2*p)*SCREEN_H/4 + round(view2[1]);
                    
                    #ifdef EMULATOR
                    write = 4;
                    #endif
                }
            }
        }  
        else 
        {
            #ifdef EMULATOR
            if (debug_draw)
                message("out of bounds\n");
            #endif
            e[i].p1.iy = 10000;
            e[i].p2.iy = 10001;
        }
    }

    // keep track of z coordinate in camera frame, in case you want to test for
    // that sort of thing.
    e[i].p1.iz = view1[2]; 
    e[i].p2.iz = view2[2]; 
    #ifdef EMULATOR
    if (debug_draw && write)
    {
        switch (write)
        {
        case 1:
            message("one z=0, one z>0\n"); 
            break;
        case 2:
            message("one z>0, one z<0\n"); 
            message("original view1: (%f, %f, %f)\n", view1copy[0], view1copy[1], view1copy[2]);
            message("original view2: (%f, %f, %f)\n", view2copy[0], view2copy[1], view2copy[2]);
            break;
        case 3:
            message("one z>0, one z=0\n"); 
            break;
        case 4:
            message("one z<0, one z>0\n"); 
            message("original view1: (%f, %f, %f)\n", view1copy[0], view1copy[1], view1copy[2]);
            message("original view2: (%f, %f, %f)\n", view2copy[0], view2copy[1], view2copy[2]);
            break;
        }
        message("e[%02d].p1.world=(%f, %f, %f), .image=(%d, %d, %f)\n", i, e[i].p1.world[0], e[i].p1.world[1], e[i].p1.world[2], e[i].p1.image[0], e[i].p1.image[1], e[i].p1.image_z);
        message("     .p2.world=(%f, %f, %f), .image=(%d, %d, %f)\n", e[i].p2.world[0], e[i].p2.world[1], e[i].p2.world[2], e[i].p2.image[0], e[i].p2.image[1], e[i].p2.image_z);
    }
    #endif

    // enforce that p1.y < p2.y
    if (e[i].p2.iy < e[i].p1.iy) // y2 is < y1
    {
        swap(&e[i].p2, &e[i].p1);
        //effort += 2;
    }
}

void graph_frame();


#endif
