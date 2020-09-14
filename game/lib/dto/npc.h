#ifndef NPC_H
#define NPC_H

#include "character.h"

struct Npc {
        struct Character character;
        int template_id;
        int server_side_name;
        char _class[200];
        double collision_radius;
        double collision_height;
        char type[20];
        int attack_range;
        int aggro;
        int r_hand_id;
        int l_hand_id;
        int armor_id;
        int faction_id;
        int faction_range;
        int is_undead;
        int absorb_level;
};

#endif