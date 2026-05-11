#pragma once
#include "../Engine/skymath.h"
#include "tilemap.h"

#define MOVE_SPEED 0.2f // NOTE: The player's movespeed measured in seconds per block.

struct EntityMap { // NOTE: Similar to a 2D tilemap, but holds entity ids instead of tile ids.
    int* map;       // NOTE: width x height x depth map. width and height matches tilemap. Can contain up to number of depth entities per tile.
    int  width;
    int  height;
    int  depth;
    int  GetID(int x, int y, int z);
    void SetID(int x, int y, int z, int id);
};
int EntityMap::GetID(int x, int y, int z) {
    return EntityMap::map[(y * EntityMap::width + x) * EntityMap::depth + z];
}
void EntityMap::SetID(int x, int y, int z, int id) {
    EntityMap::map[(y * EntityMap::width + x) * EntityMap::depth + z] = id;
}


struct EmissionTile {
    bool active;
    enum ORIENTATION_ENUM {HORIZONTAL, VERTICAL, CROSSED} orientation;
    Color color;
    Color vertical_color;
    Color horizontal_color;
};
struct EmissionMap {
    EmissionTile* map;
    bool*   orientations; // NOTE: true is up-down orientation, false is left-right
    int     width;
    int     height;
    EmissionTile  GetEmissionTile(int x, int y);
    void    SetEmissionTile(int x, int y, EmissionTile tile);
};
EmissionTile EmissionMap::GetEmissionTile(int x, int y) {
    return EmissionMap::map[y * EmissionMap::width + x];
}
void EmissionMap::SetEmissionTile(int x, int y, EmissionTile tile) {
    EmissionMap::map[y * EmissionMap::width + x] = tile;
}



// SECTION: Components
struct EntityComponentPlayer {
    enum DIRECTION_ENUM {UP, RIGHT, DOWN, LEFT, NEUTRAL} direction;
};
struct EntityComponentMover { // NOTE: Whether movable by the player
    float seconds_per_tile;
    float move_timer;
    bool  moving;
};
struct EntityComponentEmitter { // NOTE: Laser emitter struct
    Color emission_color;
    enum DIRECTION_ENUM {UP, RIGHT, DOWN, LEFT} direction;
};
struct EntityComponentReceiver { // NOTE: Laser reciever struct
    Color accepted_color;
    bool signal_received; // NOTE: turns true if a laser hits the Reciever
    bool signal_accepted; // NOTE: turns true if the laser is of the correct color.
    Color signal_color;
};
#define MAX_CONNECTED_ACTIVATORS 32
struct EntityComponentDoor { // NOTE: A door that can be open or closed
    bool is_open;
    bool open_by_default; // NOTE: If true, the door will be open by default and closed when activated
    int connected_activators_ids[MAX_CONNECTED_ACTIVATORS];
    int num_connected_activators;
};
struct EntityComponentButton {
    bool is_pressed;
};
struct EntityComponentTeleporter {
    Color color;
    int connected_teleporter_id;
};
struct EntityComponentColorChanger {
    Color color;
    enum COLOR_MODE {BLENDED, ADDITIVE, SUBTRACTIVE} color_mode;
    Color vertical_color;
    Color horizontal_color; 
};
struct EntityComponentColorGate {
    int id_of_last_changed_entity;
    enum DIRECTION_ENUM {UP, RIGHT, DOWN, LEFT} direction;
};

void EntityComponentMoverInit(EntityComponentMover* component, float seconds_per_tile, bool active = true) {
    component->active = active;
    component->seconds_per_tile = seconds_per_tile;
    component->move_timer = 0.0f;
    component->moving = false;
}
void EntityComponentEmitterInit(EntityComponentEmitter* component, Color emission_color,  EntityComponentEmitter::DIRECTION_ENUM dir, bool active = true) {
    component->active = active;
    component->emission_color = emission_color;
    component->direction = dir;
}
void EntityComponentReceiverInit(EntityComponentReceiver* component, Color accepted_color, bool active=true) {
    component->active = active;
    component->accepted_color = accepted_color; // NOTE: The color that the receiver accepts as valid.
    component->signal_received = false;
    component->signal_accepted = false;
    component->signal_color = {0, 0, 0, 0};
}
void EntityComponentDoorInit(EntityComponentDoor* component, bool open_by_default = false, int connected_receivers[MAX_CONNECTED_ACTIVATORS] = nullptr, int num_connected_activators = 0, bool active=true) {
    component->active  = active;
    component->is_open = false;
    component->open_by_default = open_by_default;
    if (num_connected_activators == 0 || connected_receivers == nullptr) {
        for (int i = 0; i < MAX_CONNECTED_ACTIVATORS; ++i) {
            component->connected_activators_ids[i] = -1; 
        }
        component->num_connected_activators = 0;
    } else {
        for (int i = 0; i < MAX_CONNECTED_ACTIVATORS; ++i) {
            if (i < num_connected_activators) {
                component->connected_activators_ids[i] = connected_receivers[i];
            } else {
                component->connected_activators_ids[i] = -1;
            }
        }
        component->num_connected_activators = num_connected_activators;
    }
}
void EntityComponentButtonInit(EntityComponentButton* component, bool is_pressed = false, bool active = true) {
    component->active = active;
    component->is_pressed = is_pressed;
}
void EntityComponentTeleporterInit(EntityComponentTeleporter* component, int connected_teleporter_id = -1, Color color = {0, 0, 0, 0}, bool active = true) {
    component->active = active;
    component->connected_teleporter_id = connected_teleporter_id;
    component->color = color;
}
void EntityComponentColorChangerInit(EntityComponentColorChanger* component, Color color = {0, 0, 0, 0}, EntityComponentColorChanger::COLOR_MODE color_mode = EntityComponentColorChanger::COLOR_MODE::ADDITIVE, bool active = true) {
    component->active = active;
    component->color = color;
    component->vertical_color = {0, 0, 0, 0};
    component->horizontal_color = {0, 0, 0, 0};
    component->color_mode = color_mode;
}

void EntityComponentColorGateInit(EntityComponentColorGate* component, bool active = true, EntityComponentColorGate::DIRECTION_ENUM dir) {
    component->active = active;
    component->id_of_last_changed_entity = -1;
    component->direction = dir;
}

// SECTION: Entities

struct Entity {
    enum ENTITY_TYPE_ENUM {PLAYER, PUSH_BLOCK, STATIC_BLOCK, EMITTER, RECEIVER, DOOR, BUTTON, ENDGOAL, TELEPORTER, COLOR_CHANGER, COLOR_GATE} entity_type;
    int         id;
    bool        active; // NOTE: This is equivalent of a null state if this is false.
    Transform   transform;
    Vector2Int  prev_position;
    Vector2Int  position;
    Color       main_color;
    int         entity_layer; // NOTE: Layer 0: floor buttons and other features. Layer 1: Pushblocks, emitters, player etc. 
    EntityComponentPlayer       player;
    EntityComponentMover        movable;
    EntityComponentEmitter      emitter;
    EntityComponentReceiver     receiver;
    EntityComponentDoor         door;
    EntityComponentButton       button;
    EntityComponentTeleporter   teleporter;
    EntityComponentColorChanger color_changer;
    EntityComponentColorGate    color_gate;
}; 


void PrintEntity(Entity e) {
    printf("############################\n");
    printf("Type:                   %d\n", e.entity_type);
    printf("ID:                     %d\n", e.id);
    printf("Active:                 %d\n", e.active);
    printf("Position:               {%d, %d}\n", e.position.x, e.position.y);
    printf("TransformPos:           {%f, %f, %f}\n", e.transform.position.x, e.transform.position.y, e.transform.position.z);
    printf("Layer:                  %d\n", e.entity_layer);
    printf("Player::Active:         %d\n", e.player.active);
    printf("Mover::Active           %d\n", e.movable.active);
    printf("Mover::Moving           %d\n", e.movable.moving);
    printf("Emitter::Active:        %d\n", e.emitter.active);
    printf("Emitter::Color:         {%d, %d, %d, %d}\n", e.emitter.emission_color.r, e.emitter.emission_color.g, e.emitter.emission_color.b, e.emitter.emission_color.a);
    printf("Receiver::Active:       %d\n", e.receiver.active);
    printf("Receiver::ColorReceived:{%d, %d, %d, %d}\n", e.receiver.signal_color.r, e.receiver.signal_color.g, e.receiver.signal_color.b, e.receiver.signal_color.a);
    printf("Receiver::ColorAccepted:{%d, %d, %d, %d}\n", e.receiver.accepted_color.r, e.receiver.accepted_color.g, e.receiver.accepted_color.b, e.receiver.accepted_color.a);
    printf("Receiver::SigReceived:  %d\n", e.receiver.signal_received);
    printf("Receiver::SigAccepted:  %d\n", e.receiver.signal_accepted);
    printf("Door::Active:           %d\n", e.door.active);
    printf("Door::Open:             %d\n", e.door.is_open);
    printf("Door::OpenByDefault:    %d\n", e.door.open_by_default);
    printf("Door::NumAttached       %d\n", e.door.num_connected_activators);
    printf("Door::AttachedReceivers:{%d", e.door.connected_activators_ids[0]);
    for (int i = 1; i < MAX_CONNECTED_ACTIVATORS; ++i) {
        
        printf(", %d", e.door.connected_activators_ids[i]);
    }
    printf("}\n");
    printf("Endgoal::Active:        %d\n", e.endgoal.active);
    printf("Button::Active:         %d\n", e.button.active);
    printf("Button::Pressed:        %d\n", e.button.is_pressed);
    printf("Teleporter::Active:     %d\n", e.teleporter.active);
    printf("Teleporter::Connected:  %d\n", e.teleporter.connected_teleporter_id);
    printf("############################\n");

} 


void EntityInit (
    Entity*     entity, 
    int         id, 
    Vector2Int  position={0, 0},
    float       entity_layer=0,
    Color       main_color = {0, 0, 0, 0},
    bool        active=true 
) {
    entity->id            = id;
    entity->active        = active;
    entity->prev_position = position; // NOTE: Cached for transform updates. Do not modify manually
    entity->position      = position;
    entity->entity_layer  = entity_layer;
    entity->main_color    = main_color;

    Transform transform = {0.0f};
    transform.position  = {(float)position.x, (float)position.y, float(entity_layer)};
    transform.rotation  = {0.0f, 0.0f, 0.0f};
    transform.scale     = {1.0f, 1.0f, 1.0f};
    
    entity->transform   = transform;

    // SECTION: Zero The entity components. Should be unnecessary but done just in case
    entity->player.active = false;
    entity->endgoal.active = false;
    EntityComponentMoverInit(&entity->movable, MOVE_SPEED, false);
    EntityComponentEmitterInit(&entity->emitter, {0, 0, 0, 0}, EntityComponentEmitter::DIRECTION_ENUM::DOWN, false);
    EntityComponentReceiverInit(&entity->receiver, {0, 0, 0, 0}, false);
    EntityComponentDoorInit(&entity->door, false, nullptr, 0, false);
    EntityComponentButtonInit(&entity->button, false, false);
    EntityComponentTeleporterInit(&entity->teleporter, -1, {0, 0, 0, 0}, false);
    EntityComponentColorChangerInit(&entity->color_changer, Color{0, 0, 0, 0}, EntityComponentColorChanger::COLOR_MODE::BLENDED, false);
    EntityComponentColorGateInit(&entity->color_gate, false, EntityComponentColorGate::DIRECTION_ENUM::UP);
}

void PlayerInit(
    Entity* player,
    int id,
    Vector2Int init_position,
    Color player_color = {255, 255, 255, 255}
) {
    EntityInit(player, id, init_position, 1.0f, player_color);
    EntityComponentMoverInit(&player->movable, MOVE_SPEED, true);
    player->player.active = true;
    player->entity_type = Entity::ENTITY_TYPE_ENUM::PLAYER;
    player->main_color = player_color;
}
void PushblockInit(
    Entity* pushblock,
    int id,
    Vector2Int init_position
) {
    EntityInit(pushblock, id, init_position, 1.0f);
    EntityComponentMoverInit(&pushblock->movable, MOVE_SPEED, true);
    pushblock->entity_type = Entity::ENTITY_TYPE_ENUM::PUSH_BLOCK;
}
void StaticBlockInit(
    Entity* static_block,
    int id,
    Vector2Int init_position
) {
    EntityInit(static_block, id, init_position, 1.0f);
    static_block->entity_type = Entity::ENTITY_TYPE_ENUM::STATIC_BLOCK;
}
void EmitterInit(
    Entity* emitter,
    int id,
    Vector2Int init_position,
    Color emitter_color,
    bool emitter_movable,
    EntityComponentEmitter::DIRECTION_ENUM direction
) {
    EntityInit(emitter, id, init_position, 1.0f, emitter_color);
    EntityComponentEmitterInit(&emitter->emitter, emitter_color, direction, true);
    EntityComponentMoverInit(&emitter->movable, MOVE_SPEED, emitter_movable);
    emitter->entity_type = Entity::ENTITY_TYPE_ENUM::EMITTER;
}
void ReceiverInit(
    Entity* receiver,
    int id,
    Vector2Int init_position,
    Color accepted_signal_color,
    bool receiver_movable
) {
    EntityInit(receiver, id, init_position, 1.0f, accepted_signal_color);
    EntityComponentReceiverInit(&receiver->receiver, accepted_signal_color, true);
    EntityComponentMoverInit(&receiver->movable, MOVE_SPEED, receiver_movable);
    receiver->entity_type = Entity::ENTITY_TYPE_ENUM::RECEIVER;
}
void DoorInit(
    Entity* door,
    int id,
    Vector2Int init_position,
    bool open_by_default = false,
    int connected_receivers[MAX_CONNECTED_ACTIVATORS] = nullptr,
    int num_connected_activators = 0
) {
    //EntityInit(door, id, sprite, init_position, 1.0f, true, false);

    EntityInit(door, id, init_position, 0.0f);
    EntityComponentDoorInit(&door->door, open_by_default, connected_receivers, num_connected_activators, true);
    door->entity_type = Entity::ENTITY_TYPE_ENUM::DOOR;
}
void EndgoalInit(
    Entity* endgoal,
    int id,
    Vector2Int init_position
) {
    EntityInit(endgoal, id, init_position, 0.0f);
    endgoal->endgoal.active = true;
    endgoal->entity_type = Entity::ENTITY_TYPE_ENUM::ENDGOAL;
}
void ButtonInit(
    Entity* button,
    int id,
    Vector2Int init_position
) {
    EntityInit(button, id,  init_position, 0.0f);
    EntityComponentButtonInit(&button->button, false, true);
    button->entity_type = Entity::ENTITY_TYPE_ENUM::BUTTON;
}
void TeleporterInit(
    Entity* teleporter,
    int id,
    Color color,
    int connected_teleporter_id,
    Vector2Int init_position
) {
    EntityInit(teleporter, id, init_position, 0.0f, color, true);
    EntityComponentTeleporterInit(&teleporter->teleporter, connected_teleporter_id, color, true);
    teleporter->entity_type = Entity::ENTITY_TYPE_ENUM::TELEPORTER;
}
void ColorChangerInit(
    Entity* color_changer,
    int id,
    Color color,
    EntityComponentColorChanger::COLOR_MODE color_mode,
    Vector2Int init_position,
    bool movable = true // Movable means its a layer 1 block, not movable is a layer 0 (can be passed through)
) {
    EntityInit(color_changer, id, init_position, movable ? 1.0f : 0.0f, color);
    EntityComponentColorChangerInit(&color_changer->color_changer, color, color_mode, true);
    EntityComponentMoverInit(&color_changer->movable, MOVE_SPEED, movable);
    color_changer->entity_type = Entity::ENTITY_TYPE_ENUM::COLOR_CHANGER;
}
void ColorGateInit(
    Entity* color_gate, 
    int id, 
    Color color,
    Vector2Int init_position
) {
    EntityInit(color_gate, id, init_position, 0.0f, color);
    EntityComponentColorGateInit(&color_gate->color_gate, true, EntityComponentColorGate::DIRECTION_ENUM::UP);
    color_gate->entity_type = Entity::ENTITY_TYPE_ENUM::COLOR_GATE; 
}



// SECTION: Entity state change functions. Should be called only when you need to change a state of the entity. NOT EVERY FRAME

// NOTE: EntityComponentMover state change
// Returns true if moving was successful. false if was unable to move the entity
bool EntityMove(int entity_id, Vector2Int direction, Tilemap map, EntityMap entity_id_map, Entity* entity_array, int move_weight = 1) {

    if (move_weight < 0) return false;
    
    Entity* entity = &entity_array[entity_id];
    
    if (!entity->movable.active) return false;

    Vector2Int new_position = entity->position + direction;

    


    if (TestTileCollide(map, new_position)) return false;

    {

        int floor_entity_id = entity_id_map.GetID(new_position.x, new_position.y, 0);
        Entity floor_entity = entity_array[floor_entity_id];
        // NOTE: Check teleporter
        if (floor_entity_id >= 0 &&
            floor_entity.active &&
            floor_entity.teleporter.active &&
            floor_entity.teleporter.connected_teleporter_id > -1
        ) {
            int teleporter_connected_id = floor_entity.teleporter.connected_teleporter_id;
            new_position = entity_array[teleporter_connected_id].position + direction;
            if (TestTileCollide(map, new_position)) return false;
        }
        
        // NOTE: Check for closed door:
        floor_entity_id = entity_id_map.GetID(new_position.x, new_position.y, 0);
        floor_entity = entity_array[floor_entity_id];
        if (floor_entity_id >= 0 && 
            floor_entity.active && 
            floor_entity.door.active && 
            !floor_entity.door.is_open
        ) {
            return false;
        } 
        
    }

    int new_entity_id = entity_id_map.GetID(new_position.x, new_position.y, entity->entity_layer);

    Entity* new_entity = &entity_array[new_entity_id]; 


    bool could_move_new_entity = false;
    
    if (new_entity_id >= 0 && new_entity->active) {
        could_move_new_entity = EntityMove(new_entity_id, direction, map, entity_id_map, entity_array, move_weight-1);
    } else if(new_entity_id < 0) {
        could_move_new_entity = true;
    } 

    if (could_move_new_entity) {
        entity->movable.moving = true;
        entity->position = new_position;

        entity_id_map.SetID(entity->position.x, entity->position.y, entity->entity_layer, entity->id);
        entity_id_map.SetID(entity->prev_position.x, entity->prev_position.y, entity->entity_layer, -1);
        
        entity->prev_position = new_position - direction;
        return true;
    }
    return false;
}

Color BlendColor(Vector4Int c, int i) {
    return Color{uint8_t(c.x / i), uint8_t(c.y / i), uint8_t(c.z / i), uint8_t(c.w / i)};
}
Color BlendColor(Color a, Color b) {
    if (a.a == 0) return b;
    else if (b.a == 0) return a;
    else return Color{uint8_t((a.r + b.r) / 2), uint8_t((a.g + b.g) / 2), uint8_t((a.b + b.b) / 2), uint8_t((a.a + b.a) / 2)};
}
Color AddColor(Color a, Color b) {
    if (b.a == 0) return a;
    if (a.a == 0) return b;
    else return a + b;
}

void EntityUpdateColorChanger(int entity_id, Entity* entity_array, EntityMap entity_id_map, EmissionMap emission_map, Tilemap map, Vector2Int direction); 
void UpdateEmit(
    int emitter_id,
    Vector2Int direction,
    Vector2Int position, 
    Color color, 
    Tilemap map, 
    EntityMap entity_id_map, 
    EmissionMap emission_map, 
    Entity* entity_array
) {
    Vector2Int new_position = position + direction;
    // NOTE: Layer 1 entities collision
    int new_entity_id = entity_id_map.GetID(new_position.x, new_position.y, 1);

    if (new_entity_id >= 0) {

        //SECTION: Activate the receiver if its hit
        Entity* new_entity = &entity_array[new_entity_id];
        if (new_entity->receiver.active) {
            // NOTE: If already recieved a signal before, blend the two colors
            if (new_entity->receiver.signal_received) new_entity->receiver.signal_color = AddColor(color, new_entity->receiver.signal_color);
            // Otherwise, just set the signal color to the one recieved and set the flag to true.
            else {
                new_entity->receiver.signal_color = color;
                new_entity->receiver.signal_received = true;
            }
        }
        // SECTION: Emit from colorchanger if it's hit
        if (new_entity->color_changer.active) {
            EntityUpdateColorChanger(new_entity_id, entity_array, entity_id_map, emission_map, map, direction);
        }

        return;
    }

    // NOTE: Layer 1 entities if moving left or right (to prevent transparency issues)
     
    //int left_entity_id = entity_id_map.GetID(IntMax(new_position.x - 1, 0), new_position.y, 1);
    //if (left_entity_id >= 0 
    //    && entity_array[left_entity_id].active 
    //    && entity_array[left_entity_id].movable.active
    //    && entity_array[left_entity_id].movable.moving
    //    && entity_array[left_entity_id].prev_position.x == new_position.x
    //) {
    //    return;
    //}
    //int right_entity_id = entity_id_map.GetID(IntMin(new_position.x + 1, entity_id_map.width-1), new_position.y, 1);
    //if (right_entity_id >= 0 
    //    && entity_array[right_entity_id].active 
    //    && entity_array[right_entity_id].movable.active
    //    && entity_array[right_entity_id].movable.moving
    //    && entity_array[right_entity_id].prev_position.x == new_position.x
    //) {
    //    return;
    //}

    //NOTE: Layer 0 (floor) entities collision (for doors when closed)
    int floor_entity_id = entity_id_map.GetID(new_position.x, new_position.y, 0);

    if (floor_entity_id >= 0) {

        
        Entity* floor_entity = &entity_array[floor_entity_id];

        if (floor_entity->active && floor_entity->door.active && !floor_entity->door.is_open) {
            return;
        }
    }


    if (TestTileCollide(map, new_position)) {
        return;
    }

    if (new_position.x < 0 || new_position.y < 0) {
        return;
    } else if (new_position.x >= map.width || new_position.y >= map.height) {
        return;
    }

    EmissionTile tile = emission_map.GetEmissionTile(new_position.x, new_position.y);
    EmissionTile::ORIENTATION_ENUM orientation = EmissionTile::HORIZONTAL;
    
    // NOTE: Since direction is in a cardinal direction, if y is not 0, then x is 0 and y is either -1 or 1.
    // Because of this, we can deduce that if direction.y is not 0, then the orientation is vertical,
    // otherwise it is horizontal
    if (direction.y != 0)        orientation = EmissionTile::VERTICAL; 
    
    
    if (tile.active) {
        if (tile.orientation != orientation) { 
            if (tile.orientation == EmissionTile::VERTICAL) {
                tile.vertical_color = tile.color;
                tile.horizontal_color = color;
            } else if (tile.orientation == EmissionTile::HORIZONTAL) {
                tile.vertical_color = color;
                tile.horizontal_color = tile.color;
            } else {
                if (orientation == EmissionTile::VERTICAL) {
                    tile.vertical_color = AddColor(tile.vertical_color, color);
                } else if (orientation == EmissionTile::HORIZONTAL) {
                    tile.horizontal_color = AddColor(tile.horizontal_color, color);
                }
            }
            tile.orientation = EmissionTile::CROSSED;
        }
        tile.color = AddColor(tile.color, color);
    } else { 
        tile.active = true;
        tile.color = color;
        tile.orientation = orientation;
        if (tile.orientation == EmissionTile::VERTICAL) tile.vertical_color = color;
        else if (tile.orientation == EmissionTile::HORIZONTAL) tile.horizontal_color = color;
    }
    emission_map.SetEmissionTile(new_position.x, new_position.y, tile);
    
    UpdateEmit(emitter_id, direction, new_position, color, map, entity_id_map, emission_map, entity_array);    
}

// NOTE: EntityComponentEmitter state change. Sets layer 2 to laser tiles.
// Returns the position of the object hit by the laser. Returns {-1,-1} if no emission can be performed
Vector2Int EntityUpdateEmit(int entity_id, Tilemap map, EntityMap entity_id_map, EmissionMap emission_map, Entity* entity_array) {
    if (entity_id < 0) return {-1, -1};

    Entity* entity = &entity_array[entity_id];

    if(!entity->active) return {-1, -1};
    if(!entity->emitter.active) return {-1, -1};     
    if(entity->movable.active && entity->movable.moving) return {-1, -1}; // NOTE: do not emit if the emitter is moving

    Vector2Int direction = {0, 0};

    if (entity->emitter.direction == EntityComponentEmitter::DIRECTION_ENUM::UP) {
        direction.y = 1;
    } else if (entity->emitter.direction == EntityComponentEmitter::DIRECTION_ENUM::LEFT) {
        direction.x = -1;
    } else if (entity->emitter.direction == EntityComponentEmitter::DIRECTION_ENUM::DOWN) {
        direction.y = -1;
    } else if (entity->emitter.direction == EntityComponentEmitter::DIRECTION_ENUM::RIGHT) {
        direction.x = 1;
    }
    UpdateEmit(entity_id, direction, entity->position, entity->emitter.emission_color, map, entity_id_map, emission_map, entity_array);
    return {-1, -1};
}


// SECTION: Update methods. Should be called every frame 
void EntityUpdateMover(int entity_id, Entity* entity_array, float dt) {
    Entity* entity = &entity_array[entity_id];

    if (!entity->active) return;
    if (!entity->movable.active) return;
    if (entity->movable.moving) entity->movable.move_timer += dt;
    float move_t = entity->movable.move_timer / (entity->movable.seconds_per_tile * 1000);

    entity->transform.position.y = Lerp((float)entity->prev_position.y, (float)entity->position.y, move_t);
    entity->transform.position.x = Lerp((float)entity->prev_position.x, (float)entity->position.x, move_t);
    //entity->transform.position.y = Lerp(entity->transform.position.y, (float)entity->position.y, move_t);
    //entity->transform.position.x = Lerp(entity->transform.position.x, (float)entity->position.x, move_t);

    if (move_t >= 1.0f) {
        entity->transform.position.y = (float)entity->position.y;
        entity->transform.position.x = (float)entity->position.x;


        entity->movable.move_timer = 0.0f;
        entity->movable.moving = false;
        entity->prev_position = entity->position;
    }
}

void EntityUpdateReceiver(int entity_id, Entity* entity_array) {
    Entity* entity = &entity_array[entity_id];

    if (!entity->active || !entity->receiver.active) return;

    if (entity->receiver.signal_received) {
        if (CompareColorWithTolerance(entity->receiver.signal_color, entity->receiver.accepted_color, 2)) {
            entity->receiver.signal_accepted = true;
        } else {
            entity->receiver.signal_accepted = false;
        }
        //entity->receiver.signal_accepted = true;
    } else if (!entity->receiver.signal_received) {
        entity->receiver.signal_color = {0, 0, 0, 0};
        entity->receiver.signal_accepted = false;
    }

}

void EntityUpdateDoor(int entity_id, Entity* entity_array, EntityMap map) {
    Entity* door_entity = &entity_array[entity_id];

    if (!door_entity->active || 
        !door_entity->door.active 
    ) return;

    bool open_by_default = door_entity->door.open_by_default;
    door_entity->door.is_open = open_by_default;

    if (door_entity->door.num_connected_activators > 0) { 
        for (int i = 0; i < door_entity->door.num_connected_activators; ++i) {
            int id = door_entity->door.connected_activators_ids[i];
            if (id < 0) continue;
            Entity* activator_entity = &entity_array[id];
            
            if (!activator_entity->active) continue;

            if (activator_entity->receiver.active && activator_entity->receiver.signal_accepted) {
                door_entity->door.is_open = !open_by_default;
                break;
            } else if (activator_entity->button.active && activator_entity->button.is_pressed) {
                door_entity->door.is_open = !open_by_default;
                break;
            }
        }
    }
    int top_entity_id = map.GetID(door_entity->position.x, door_entity->position.y, 1);
    if (top_entity_id >= 0 && entity_array[top_entity_id].active && top_entity_id != door_entity->id) door_entity->door.is_open = true;

    if (!door_entity->door.is_open) {
        map.SetID(door_entity->position.x, door_entity->position.y, 1, door_entity->id);
    } else if (door_entity->door.is_open) {
        if (top_entity_id == door_entity->id) {
            map.SetID(door_entity->position.x, door_entity->position.y, 1, -1);
        }
    }
}

void EntityUpdateButton(int entity_id, Entity* entity_array, EntityMap map) {

    Entity* entity = &entity_array[entity_id];

    if (!entity->active || !entity->button.active) return;


    Vector2Int pos = entity->position;
    int top_entity_id = map.GetID(pos.x, pos.y, 1);

    if (top_entity_id < 0) entity->button.is_pressed = false;
    else entity->button.is_pressed = true;
}

void EntityUpdateColorChanger(int entity_id, Entity* entity_array, EntityMap entity_id_map, EmissionMap emission_map, Tilemap map, Vector2Int direction) {
    if (entity_id < 0) return;
    Entity* entity = &entity_array[entity_id];
    
    if (!entity->active || !entity->color_changer.active) return;

    int x_pos = entity->position.x;
    int y_pos = entity->position.y;

    bool is_vertical = false;

    EmissionTile::ORIENTATION_ENUM not_orientation;
    EntityComponentEmitter::DIRECTION_ENUM emitter_direction;

    if (direction.x < 0) { // Left
        x_pos++;
        not_orientation = EmissionTile::ORIENTATION_ENUM::VERTICAL;
        emitter_direction = EntityComponentEmitter::DIRECTION_ENUM::LEFT;
    } else if (direction.x > 0) { // Right
        x_pos--;
        not_orientation = EmissionTile::ORIENTATION_ENUM::VERTICAL;
        emitter_direction = EntityComponentEmitter::DIRECTION_ENUM::RIGHT;
    } else if (direction.y < 0) { // UP
        y_pos++;
        not_orientation = EmissionTile::ORIENTATION_ENUM::HORIZONTAL;
        emitter_direction = EntityComponentEmitter::DIRECTION_ENUM::DOWN;
        is_vertical = true;
    } else if (direction.y > 0) { // DOWN
        y_pos--;
        not_orientation = EmissionTile::ORIENTATION_ENUM::HORIZONTAL;
        emitter_direction = EntityComponentEmitter::DIRECTION_ENUM::UP;
        is_vertical = true;
    }

    EmissionTile prev_tile = emission_map.GetEmissionTile(x_pos, y_pos);
    int prev_entity_id = entity_id_map.GetID(x_pos, y_pos, 1);
    

    // If an emission is above and is vertical or crossed (not horizontal)
    if (
        (prev_tile.active && prev_tile.orientation != not_orientation) ||
        (
            prev_entity_id >= 0 &&
            entity_array[prev_entity_id].active && 
            entity_array[prev_entity_id].emitter.active && 
            entity_array[prev_entity_id].emitter.direction == emitter_direction
        ) ||
        (
            prev_entity_id >= 0 &&
            entity_array[prev_entity_id].active &&
            entity_array[prev_entity_id].color_changer.active 
        ) 
    ) {
        Entity prev_entity = entity_array[prev_entity_id];
        Color new_color;
        if (prev_tile.active) 
            new_color = BlendColor(is_vertical ? prev_tile.vertical_color : prev_tile.horizontal_color, entity->color_changer.color);
        else if (prev_entity.active && prev_entity.emitter.active) 
            new_color = BlendColor(prev_entity.emitter.emission_color, entity->color_changer.color);
        else if (prev_entity.active && prev_entity.color_changer.active) 
            new_color = BlendColor(is_vertical ? prev_entity.color_changer.vertical_color : prev_entity.color_changer.horizontal_color, entity->color_changer.color);
        if (is_vertical) entity->color_changer.vertical_color = new_color;
        else entity->color_changer.horizontal_color = new_color;
        UpdateEmit(entity_id, direction, entity->position, new_color, map, entity_id_map, emission_map, entity_array);
    } 
 
}


// SECTION: Render methods. Should be called after update methods when you actually do the rendering.
void EntityRender(int entity_id, Entity* entity_array, GL_ID* shaders, const Sprite* sprites, bool rendering_top = true, bool level_transitioning = false) {
    Entity* entity = &entity_array[entity_id];

    if (!entity->active) return;

    // NOTE: Below is just for rendering purposes. It brings moving entities to front for when they cross
    // from higher y to lower y. Without it, the moving entity gets rendering behind any floor entities of
    // that have a higher y then them (since distance from camera depends on y.)
    // NOTE 2: This increment to z position is undone at the end of the function with the same conditional.
    if (entity->movable.active && entity->movable.moving) {
        if (entity->prev_position.y != entity->position.y)
            entity->transform.position.z += 1.3f;
    }

    if (entity->player.active) {
        switch(entity->player.direction) {
            case EntityComponentPlayer::DIRECTION_ENUM::UP: {
                Transform t = {0.0f};
                CopyTransform(&t, entity->transform);
                if (rendering_top) {
                    t.position.y += 0.5f;
                    
                    ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
                    ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 0.5f});
                    ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.5f});
                    
                    DrawSprite(sprites[1], t, main_camera);
                    t.position.z += 0.05f;
                    if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vec4(entity->main_color));
                    DrawSprite(sprites[26], t, main_camera);
                    if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vector4{1.0f, 1.0f, 1.0f, 1.0f});
                    
                    ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
                    ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 1.0f});
                    ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.0f});

                    
                } else {
                    t.position.y -= 0.5f;
                    
                    ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
                    ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 0.5f});
                    
                    DrawSprite(sprites[1], t, main_camera);
                    t.position.z += 0.05f;
                    if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vec4(entity->main_color));
                    DrawSprite(sprites[26], t, main_camera);
                    if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vector4{1.0f, 1.0f, 1.0f, 1.0f});
                    
                    ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
                    ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 1.0f});
                }
                return;
            } break;
            case EntityComponentPlayer::DIRECTION_ENUM::DOWN: {
                Transform t = {0.0f};
                CopyTransform(&t, entity->transform);
                if (rendering_top) {
                    t.position.y += 0.5f;
                    
                    ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
                    ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 0.5f});
                    ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.5f});
                    
                    DrawSprite(sprites[2], t, main_camera);
                    t.position.z += 0.05f;
                    if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vec4(entity->main_color));
                    DrawSprite(sprites[27], t, main_camera);
                    if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vector4{1.0f, 1.0f, 1.0f, 1.0f});
                    
                    ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
                    ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 1.0f});
                    ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.0f});
                } else {
                    t.position.y -= 0.5f;
                    
                    ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
                    ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 0.5f});
                    
                    DrawSprite(sprites[2], t, main_camera);
                    t.position.z += 0.05f;
                    if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vec4(entity->main_color));
                    DrawSprite(sprites[27], t, main_camera);
                    if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vector4{1.0f, 1.0f, 1.0f, 1.0f});
                    
                    ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
                    ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 1.0f});
                }
                return;
            } break;
            case EntityComponentPlayer::DIRECTION_ENUM::LEFT: {
                Transform t = {0.0f};
                CopyTransform(&t, entity->transform);
                if (rendering_top) {
                    t.position.y += 0.5f;
                    
                    ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
                    ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 0.5f});
                    ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.5f});
                    
                    DrawSprite(sprites[3], t, main_camera);
                    t.position.z += 0.05f;
                    if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vec4(entity->main_color));
                    DrawSprite(sprites[28], t, main_camera);
                    if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vector4{1.0f, 1.0f, 1.0f, 1.0f});
                    
                    ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
                    ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 1.0f});
                    ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.0f});
                } else {
                    t.position.y -= 0.5f;

                    ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
                    ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 0.5f});
                    
                    DrawSprite(sprites[3], t, main_camera);
                    t.position.z += 0.05f;
                    if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vec4(entity->main_color));
                    DrawSprite(sprites[28], t, main_camera);
                    if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vector4{1.0f, 1.0f, 1.0f, 1.0f});
                    
                    ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
                    ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 1.0f});
                }
                return;
            } break;
            case EntityComponentPlayer::DIRECTION_ENUM::RIGHT: {
                Transform t = {0.0f};
                CopyTransform(&t, entity->transform);
                if (rendering_top) {
                    t.position.y += 0.5f;
                    
                    ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
                    ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 0.5f});
                    ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.5f});
                    
                    DrawSprite(sprites[4], t, main_camera);
                    t.position.z += 0.05f;
                    if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vec4(entity->main_color));
                    DrawSprite(sprites[29], t, main_camera);
                    if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vector4{1.0f, 1.0f, 1.0f, 1.0f});
                    
                    ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
                    ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 1.0f});
                    ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.0f});
                } else {
                    t.position.y -= 0.5f;

                    ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
                    ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 0.5f});
                    
                    DrawSprite(sprites[4], t, main_camera);
                    t.position.z += 0.05f;
                    if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vec4(entity->main_color));
                    DrawSprite(sprites[29], t, main_camera);
                    if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vector4{1.0f, 1.0f, 1.0f, 1.0f});
                    
                    ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
                    ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 1.0f});
                }
                return;
            } break;
            case EntityComponentPlayer::DIRECTION_ENUM::NEUTRAL: {
                Transform t = {0.0f};
                CopyTransform(&t, entity->transform);
                if (rendering_top) {
                    t.position.y += 0.5f;
                    
                    ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
                    ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 0.5f});
                    ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.5f});
                    
                    DrawSprite(sprites[0], t, main_camera);
                    t.position.z += 0.05f;
                    if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vec4(entity->main_color));
                    DrawSprite(sprites[25], t, main_camera);
                    if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vector4{1.0f, 1.0f, 1.0f, 1.0f});
                    
                    ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
                    ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 1.0f});
                    ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.0f});
                } else {
                    t.position.y -= 0.5f;
                    
                    ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
                    ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 0.5f});
                    
                    DrawSprite(sprites[0], t, main_camera);
                    t.position.z += 0.05f;
                    if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vec4(entity->main_color));
                    DrawSprite(sprites[25], t, main_camera);
                    if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vector4{1.0f, 1.0f, 1.0f, 1.0f});
                    
                    ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
                    ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 1.0f});
                }
                return;
            } break;
        }
    }


    // SECTION: Rendering door
    else if (entity->door.active) {
        Transform transform = entity->transform;
        if (entity->door.is_open) {
            transform.scale.y = 1.0f;
            if (entity->door.open_by_default) {
                DrawSprite(sprites[15], transform, main_camera);
            } else {
                DrawSprite(sprites[13], transform, main_camera);
            }
        } else {
            if (entity->door.open_by_default) {
                Transform t = {0.0f};
                CopyTransform(&t, entity->transform);
                if (rendering_top) {
                    t.position.y += 0.5f;
                    
                    ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
                    ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 0.5f});
                    ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.5f});
                    
                    DrawSprite(sprites[16], t, main_camera);
                    
                    ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
                    ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 1.0f});
                    ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.0f});
                } else {
                    t.position.y -= 0.5f;
                    
                    ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
                    ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 0.5f});
                    
                    DrawSprite(sprites[16], t, main_camera);
                    
                    ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
                    ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 1.0f});
                }
            } else {
                Transform t = {0.0f};
                CopyTransform(&t, entity->transform);
                if (rendering_top) {
                    t.position.y += 0.5f;
                    
                    ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
                    ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 0.5f});
                    ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.5f});
                    
                    DrawSprite(sprites[14], t, main_camera);
                    
                    ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
                    ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 1.0f});
                    ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.0f});
                } else {
                    t.position.y -= 0.5f;
                    
                    ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
                    ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 0.5f});
                    
                    DrawSprite(sprites[14], t, main_camera);
                    
                    ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
                    ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 1.0f});
                }
            }
        }
    }

    else if (entity->button.active) {
        Transform transform = entity->transform;
        if (entity->button.is_pressed) {
            DrawSprite(sprites[19], transform, main_camera);
        } else {
            DrawSprite(sprites[18], transform, main_camera);
        }

    }

    // SECTION: Emitter rendering
    else if (entity->emitter.active) {
        Transform t = {0.0f};
        CopyTransform(&t, entity->transform);
        if (rendering_top) {
            t.position.y += 0.5f;
            
            ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
            ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 0.5f});
            ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.5f});
            
            DrawSprite(sprites[7], t, main_camera);
            
            ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
            ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 1.0f});
            ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.0f});
        } else {
            t.position.y -= 0.5f;
            
            ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
            ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 0.5f});
            
            DrawSprite(sprites[7], t, main_camera);
            
            ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
            ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 1.0f});
        }

        t.position.z += 0.1f;
        if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vec4(entity->emitter.emission_color));
        if (rendering_top) {
            
            ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
            ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 0.5f});
            ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.5f});
            
            DrawSprite(sprites[8], t, main_camera);
            
            ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
            ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 1.0f});
            ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.0f});
        } else {
            
            ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
            ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 0.5f});
            
            DrawSprite(sprites[8], t, main_camera);
            
            ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
            ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 1.0f});
        }
        if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vector4{1.0f, 1.0f, 1.0f, 1.0f});
    }
    // SECTION: Receiver rendering
    else if (entity->receiver.active) {
        Transform t = {0.0f};
        CopyTransform(&t, entity->transform);
        if (rendering_top) {
            t.position.y += 0.5f;
            
            ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
            ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 0.5f});
            ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.5f});
            
            DrawSprite(sprites[10], t, main_camera);
            
            ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
            ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 1.0f});
            ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.0f});
        } else {
            t.position.y -= 0.5f;
            
            ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
            ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 0.5f});
            
            DrawSprite(sprites[10], t, main_camera);
            
            ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
            ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 1.0f});
        }
        t.position.z += 0.1f;
        if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vec4(entity->receiver.accepted_color));
        if (rendering_top) {
            ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
            ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 0.5f});
            ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.5f});
            
            DrawSprite(sprites[12], t, main_camera);
            
            ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
            ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 1.0f});
            ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.0f});
        } else {
            ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
            ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 0.5f});
            
            DrawSprite(sprites[12], t, main_camera);
            
            ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
            ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 1.0f});
        }
        if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vector4{1.0f, 1.0f, 1.0f, 1.0f});
        t.position.z += 0.1f;
        if (entity->receiver.signal_received) {
            if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vec4(entity->receiver.signal_color));
            if (rendering_top) {
                ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
                ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 0.5f});
                ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.5f});
                
                DrawSprite(sprites[11], t, main_camera);
                
                ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
                ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 1.0f});
                ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.0f});
            } else {
                ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
                ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 0.5f});
                
                DrawSprite(sprites[11], t, main_camera);
                
                ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
                ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 1.0f});
            }
            //DrawSprite(sprites[11], t, main_camera);
        }
        if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vector4{1.0f, 1.0f, 1.0f, 1.0f});
        // NOTE: now that we finished rendering, we can mark receiver non-signaled.
        if (rendering_top) entity->receiver.signal_received = false;
    } else if (entity->teleporter.active) {
        if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vec4(entity->teleporter.color));
        DrawSprite(sprites[20], entity->transform, main_camera);
        if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vector4{1.0f, 1.0f, 1.0f, 1.0f});
    } else if (entity->color_changer.active) {
        Transform t = {0.0f};
        CopyTransform(&t, entity->transform);
        // NOTE: Draw the transparent part (main object)
        if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vec4(entity->color_changer.color));
        if (rendering_top) {
            t.position.y += 0.5f;
            
            ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
            ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 0.5f});
            ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.5f});
            
            DrawSprite(sprites[21], t, main_camera);
            
            ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
            ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 1.0f});
            ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.0f});
        } else {
            t.position.y -= 0.5f;
            
            ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
            ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 0.5f});
            
            DrawSprite(sprites[21], t, main_camera);
            
            ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
            ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 1.0f});
        }
        if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vector4{1.0f, 1.0f, 1.0f, 1.0f});
        // NOTE: Draw the frame
        t.position.z += 0.1f;
        if (rendering_top) {
            ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
            ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 0.5f});
            ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.5f});
            
            DrawSprite(sprites[22], t, main_camera);
            
            ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
            ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 1.0f});
            ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.0f});
        } else {
            ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
            ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 0.5f});
            
            DrawSprite(sprites[22], t, main_camera);
            
            ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
            ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 1.0f});
        }

        if (!rendering_top) return;
        // NOTE: Draw all the blended lasers
        ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
        ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f/5.0f, 1.0f});
        ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.0f});
        t.scale.y = 2.0f;
        t.position.y -= 0.5f;
        t.position.z += 0.1f;
        if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vec4(entity->color_changer.horizontal_color));
        DrawSprite(sprites[23], t, main_camera);
        
        ShaderSetVector(shaders, "uv_offset", Vector2{1.0f/5.0f, 0.0f});
        t.position.z += 0.1f;
        if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vec4(entity->color_changer.vertical_color));
        DrawSprite(sprites[23], t, main_camera);
        

        ShaderSetVector(shaders, "uv_offset", Vector2{2.0f/5.0f, 0.0f});
        if (entity->color_changer.vertical_color.a == 0) ShaderSetVector(shaders, "uv_offset", Vector2{3.0f/5.0f, 0.0f});
        else if (entity->color_changer.horizontal_color.a == 0) ShaderSetVector(shaders, "uv_offset", Vector2{4.0f/5.0f, 0.0f});
        t.position.z += 0.1f;
        if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vec4(AddColor(entity->color_changer.horizontal_color, entity->color_changer.vertical_color)));
        DrawSprite(sprites[23], t, main_camera);
        ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
        ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 1.0f});
        ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.0f});
        if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vector4{1.0f, 1.0f, 1.0f, 1.0f});

        entity->color_changer.vertical_color = {0, 0, 0, 0};
        entity->color_changer.horizontal_color = {0, 0, 0, 0};

    } else if (entity->color_gate.active) {
        if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vec4(entity->main_color));
        DrawSprite(sprites[24], entity->transform, main_camera);
        if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vector4{1.0f, 1.0f, 1.0f, 1.0f});
    }
        // NOTE: Handle pushblock, static block, endgoal
    else if (entity->endgoal.active) {
        DrawSprite(sprites[17], entity->transform, main_camera);
    } else if (entity->movable.active) {
        Transform t = {0.0f};
        CopyTransform(&t, entity->transform);
        if (rendering_top) {
            t.position.y += 0.5f;
            
            ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
            ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 0.5f});
            ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.5f});
            
            DrawSprite(sprites[5], t, main_camera);
            
            ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
            ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 1.0f});
            ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.0f});
        } else {
            t.position.y -= 0.5f;
            
            ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
            ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 0.5f});
            
            DrawSprite(sprites[5], t, main_camera);
            
            ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
            ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 1.0f});
        }
    } else {
        Transform t = {0.0f};
        CopyTransform(&t, entity->transform);
        if (rendering_top) {
            t.position.y += 0.5f;
            
            ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
            ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 0.5f});
            ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.5f});
            
            DrawSprite(sprites[6], t, main_camera);
            
            ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
            ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 1.0f});
            ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.0f});
        } else {
            t.position.y -= 0.5f;
            
            ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
            ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 0.5f});
            
            DrawSprite(sprites[6], t, main_camera);
            
            ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
            ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 1.0f});
        }
    }



    // NOTE: Below is just for rendering purposes. It brings moving entities to front for when they cross
    // from higher y to lower y. Without it, the moving entity gets rendering behind any floor entities of
    // that have a higher y then them (since distance from camera depends on y.)
    if (entity->movable.active && entity->movable.moving) {
        if (entity->prev_position.y != entity->position.y)
            entity->transform.position.z -= 1.1f;
    }
}
void EmissionRender(int x, int y, EmissionMap map, Sprite emission_sprite_sheet, GL_ID* shaders, bool level_transitioning = false) {
    
    EmissionTile tile = map.GetEmissionTile(x, y);
    if (tile.active == false) return;

    float uv_x_offset = 0.0f;
    float scale_x = 1.0f;
    float scale_y = (6.0f/16.0f);
    if (tile.orientation == EmissionTile::VERTICAL) { 
        uv_x_offset = (1.0f/5.0f);
        scale_x = scale_y;
        scale_y = 1.0f;
    }
    if (tile.orientation == EmissionTile::CROSSED) { 
        uv_x_offset = (2.0f/5.0f);
        scale_x = (6.0f/16.0f);
    }
    
    ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
    ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f/5.0f, 1.0f});
    ShaderSetVector(shaders, "uv_offset", Vector2{uv_x_offset, 0.0f});
    
    Transform transform = {0.0f};
    transform.position = Vector3{float(x), float(y+0.5f), 0.5f - (2*y)};
    transform.rotation = Vector3{0.0f, 0.0f, 0.0f};
    transform.scale    = Vector3{scale_x, scale_y, 1.0f}; 
    
    if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vec4(map.GetEmissionTile(x,y).color));
    DrawSprite(emission_sprite_sheet, transform, main_camera);
    
    if (tile.orientation == EmissionTile::CROSSED) {
        transform.scale.y = (6.0f/16.0f);
        transform.scale.x = 1.0f;
        transform.position.z += 0.1f;
        uv_x_offset = (3.0f/5.0f);
        ShaderSetVector(shaders, "uv_offset", Vector2{uv_x_offset, 0.0f});
        if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vec4(map.GetEmissionTile(x,y).horizontal_color));
        DrawSprite(emission_sprite_sheet, transform, main_camera);

        transform.scale.y = 1.0f;
        transform.scale.x = (6.0f/16.0f);
        transform.position.z += 0.1f;
        uv_x_offset = (4.0f/5.0f);
        ShaderSetVector(shaders, "uv_offset", Vector2{uv_x_offset, 0.0f});
        if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vec4(map.GetEmissionTile(x,y).vertical_color));
        DrawSprite(emission_sprite_sheet, transform, main_camera);
    }

    
    
    ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
    ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 1.0f});
    ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.0f});
    if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vector4{1.0f, 1.0f, 1.0f, 1.0f});

    tile = {false, EmissionTile::HORIZONTAL, Color{0, 0, 0, 0}, Color{0, 0, 0, 0}, Color{0, 0, 0, 0}};
    map.SetEmissionTile(x, y, tile);

}