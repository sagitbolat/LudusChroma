#pragma once

#include "../Engine/sky_structs.h"

#include "entity.h"
Color hidden_color_array[] = {
    DEFAULT_BLACK,
    DEFAULT_RED,
    DEFAULT_YELLOW,
    DEFAULT_GREEN,
    DEFAULT_CYAN,
    DEFAULT_BLUE,
    DEFAULT_MAGENTA
};

uint8_t curr_hidden_color = 0;

#define MAX_SHAKE_ENTRIES 16
const float SHAKE_DURATION = 400.f;
struct ShakeEntry { int entity_id; float timer; };
ShakeEntry shake_entries[MAX_SHAKE_ENTRIES] = {};

// Returns true if the switch can proceed (no conflicts). Adds shakes for blockers.
bool CheckHiddenColorSwitch(EntityMap* entity_map, ComponentArrays* component_arrays) {
    int prev_color    = curr_hidden_color;
    bool has_conflict = false;
    for (const int entity_id : component_arrays->color_tag_arr.dense_ids) {
        ColorTag*     ct = component_arrays->color_tag_arr.Get(entity_id);
        GridPosition* gp = component_arrays->grid_position_arr.Get(entity_id);
        if (!gp) continue;
        if (ct->color != hidden_color_array[prev_color]) continue;
        int occupant = entity_map->GetID(gp->position, (int)gp->layer);
        if (occupant >= 0 && occupant != entity_id) {
            has_conflict = true;
            for (int s = 0; s < MAX_SHAKE_ENTRIES; ++s) {
                if (shake_entries[s].timer <= 0.f) {
                    shake_entries[s] = { occupant, SHAKE_DURATION };
                    break;
                }
            }
        }
    }
    return !has_conflict;
}

void CommitHiddenColorSwitch(EntityMap* entity_map, ComponentArrays* component_arrays) {
    int prev_color = curr_hidden_color;
    int next_color = (curr_hidden_color + 1) % 7;
    curr_hidden_color = next_color;

    for (const int entity_id : component_arrays->color_tag_arr.dense_ids) {
        ColorTag*     ct = component_arrays->color_tag_arr.Get(entity_id);
        GridPosition* gp = component_arrays->grid_position_arr.Get(entity_id);
        if (!gp) continue;

        bool was_hidden = (ct->color == hidden_color_array[prev_color]);
        bool is_hidden  = (ct->color == hidden_color_array[next_color]);

        if (was_hidden && !is_hidden) {
            entity_map->SetID(gp->position, (int)gp->layer, entity_id);
        } else if (!was_hidden && is_hidden) {
            int occupant = entity_map->GetID(gp->position, (int)gp->layer);
            if (occupant == entity_id)
                entity_map->SetID(gp->position, (int)gp->layer, -1);
        }
    }
}

bool isHidden(int entity_id, ComponentArrays* component_arrays) {
    ColorTag* color_tag = component_arrays->color_tag_arr.Get(entity_id);
    if (color_tag == nullptr) return false;
    return (color_tag->color == hidden_color_array[curr_hidden_color]);
}