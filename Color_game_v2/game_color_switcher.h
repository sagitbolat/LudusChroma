#pragma once

#include "../Engine/sky_structs.h"

#include "entity.h"
Color hidden_color_array[] = {
    DEFAULT_BLACK,
    DEFAULT_RED,
    DEFAULT_GREEN,
    DEFAULT_BLUE/*,
    DEFAULT_YELLOW,
    DEFAULT_MAGENTA,
    DEFAULT_CYAN*/
};

uint8_t curr_hidden_color = 0;

void SwitchHiddenColor(EntityMap* entity_map, ComponentArrays* component_arrays) {

    int prev_hidden_color = curr_hidden_color;
    curr_hidden_color = (curr_hidden_color + 1) % 4; // TODO: change this to 6

    for (const int entity_id : component_arrays->color_tag_arr.dense_ids) {
        ColorTag* color_tag = component_arrays->color_tag_arr.Get(entity_id);
        GridPosition* grid_position = component_arrays->grid_position_arr.Get(entity_id);

        bool was_hidden = (color_tag->color == hidden_color_array[prev_hidden_color]);
        bool is_hidden = (color_tag->color == hidden_color_array[curr_hidden_color]);

        // Logic path for un-hiding entities
        if (was_hidden && !is_hidden) {
            int existing_entity_id = entity_map->GetID(grid_position->position, (int)grid_position->layer);

            if (existing_entity_id == -1) {
                // NOTE: No merge conflict at the cell
                entity_map->SetID(grid_position->position, (int)grid_position->layer, entity_id);
            } else {
                // NOTE: Merge conflict, don't insert and abadon the color switch entirely.
                // TODO: Maybe play around with what to do on merge conflict.
                curr_hidden_color = prev_hidden_color;
                return;
            }
        }
        // Logic Path for hiding entities
        if (!was_hidden && is_hidden) {
            int existing_entity_id = entity_map->GetID(grid_position->position, (int)grid_position->layer);
            
            // NOTE: this should always fire.
            if (existing_entity_id == entity_id)
                entity_map->SetID(grid_position->position, (int)grid_position->layer, -1);
        }


    }

}

bool isHidden(int entity_id, ComponentArrays* component_arrays) {
    ColorTag* color_tag = component_arrays->color_tag_arr.Get(entity_id);
    if (color_tag == nullptr) return false;
    return (color_tag->color == hidden_color_array[curr_hidden_color]);
}