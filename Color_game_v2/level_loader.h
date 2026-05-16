#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "tilemap.h"
#include "entity.h"

// ============================================================
// V2 Level File Format
// ============================================================
// Header:   [width u32][height u32][num_floor_types u32][num_entities u32]
// Tilemap:  width * height  uint16 tile values
// Entities: num_entities records, each:
//   [entity_type u32][pos_x u32][pos_y u32]
//   ...type-specific fields (see EntityTypeV2 below)
//
// Signal wiring: doors/receivers/buttons share integer channel IDs.
// An activator and a door are connected when they share at least one channel ID.
// ============================================================

const char level_directory[]       = "levels";
const char level_filetype_extension[] = "level";

enum class EntityTypeV2 : uint32_t {
    Player       = 0,
    PushBlock    = 1,
    StaticBlock  = 2,
    Emitter      = 3,
    Receiver     = 4,
    Door         = 5,
    Endgoal      = 6,
    Button       = 7,
    Teleporter   = 8,
    ColorChanger = 9,
};

struct LevelStateInfo {
    int num_floor_tile_types;
    int num_entities;
};


// ============================================================
// Low-level I/O helpers
// ============================================================

static inline void WriteU32(uint32_t v, FILE* f) { fwrite(&v, sizeof(v), 1, f); }
static inline uint32_t ReadU32(FILE* f) { uint32_t v = 0; fread(&v, sizeof(v), 1, f); return v; }

static inline void WriteColor(Color c, FILE* f) {
    WriteU32(c.r, f); WriteU32(c.g, f); WriteU32(c.b, f); WriteU32(c.a, f);
}
static inline Color ReadColor(FILE* f) {
    Color c{};
    c.r = (uint8_t)ReadU32(f);
    c.g = (uint8_t)ReadU32(f);
    c.b = (uint8_t)ReadU32(f);
    c.a = (uint8_t)ReadU32(f);
    return c;
}

static inline void WriteChannels(const int32_t channels[MAX_CONNECTIONS], FILE* f) {
    for (int i = 0; i < MAX_CONNECTIONS; ++i) WriteU32((uint32_t)channels[i], f);
}
static inline void ReadChannels(int32_t channels[MAX_CONNECTIONS], FILE* f) {
    for (int i = 0; i < MAX_CONNECTIONS; ++i) channels[i] = (int32_t)ReadU32(f);
}


// ============================================================
// Save
// ============================================================

// Write entity type + position + type-specific component data.
// comp_arrays is queried by entity_id to find which components exist.
static void WriteEntityRecord(int entity_id, ComponentArrays* ca, FILE* f) {
    GridPosition* gp = ca->grid_position_arr.Get(entity_id);
    if (!gp) return;

    Vector2Int pos = gp->position;

    // Determine type by component presence (priority matches game semantics)
    if (ca->grid_player_controlled_arr.Get(entity_id)) {
        GridPlayerControlled* pc = ca->grid_player_controlled_arr.Get(entity_id);
        WriteU32((uint32_t)EntityTypeV2::Player, f);
        WriteU32((uint32_t)pos.x, f); WriteU32((uint32_t)pos.y, f);
        WriteColor(pc->color, f);
        WriteU32((uint32_t)pc->orientation, f);
    } else if (ca->door_arr.Get(entity_id)) {
        Door*          door = ca->door_arr.Get(entity_id);
        SignalChannel* sc   = ca->signal_channel_arr.Get(entity_id);
        int32_t empty[MAX_CONNECTIONS]; for (int i=0;i<MAX_CONNECTIONS;++i) empty[i]=-1;
        WriteU32((uint32_t)EntityTypeV2::Door, f);
        WriteU32((uint32_t)pos.x, f); WriteU32((uint32_t)pos.y, f);
        WriteU32(door->open_by_default ? 1u : 0u, f);
        WriteChannels(sc ? sc->channels : empty, f);
    } else if (ca->button_arr.Get(entity_id)) {
        SignalChannel* sc = ca->signal_channel_arr.Get(entity_id);
        int32_t empty[MAX_CONNECTIONS]; for (int i=0;i<MAX_CONNECTIONS;++i) empty[i]=-1;
        WriteU32((uint32_t)EntityTypeV2::Button, f);
        WriteU32((uint32_t)pos.x, f); WriteU32((uint32_t)pos.y, f);
        WriteChannels(sc ? sc->channels : empty, f);
    } else if (ca->laser_emitter_arr.Get(entity_id)) {
        LaserEmitter* le = ca->laser_emitter_arr.Get(entity_id);
        WriteU32((uint32_t)EntityTypeV2::Emitter, f);
        WriteU32((uint32_t)pos.x, f); WriteU32((uint32_t)pos.y, f);
        WriteColor(le->color, f);
        WriteU32((uint32_t)le->dir, f);
    } else if (ca->laser_receiver_arr.Get(entity_id)) {
        LaserReceiver* lr = ca->laser_receiver_arr.Get(entity_id);
        SignalChannel* sc = ca->signal_channel_arr.Get(entity_id);
        int32_t empty[MAX_CONNECTIONS]; for (int i=0;i<MAX_CONNECTIONS;++i) empty[i]=-1;
        WriteU32((uint32_t)EntityTypeV2::Receiver, f);
        WriteU32((uint32_t)pos.x, f); WriteU32((uint32_t)pos.y, f);
        WriteColor(lr->accepted_color, f);
        WriteChannels(sc ? sc->channels : empty, f);
    } else if (ca->teleporter_arr.Get(entity_id)) {
        Teleporter* tp = ca->teleporter_arr.Get(entity_id);
        WriteU32((uint32_t)EntityTypeV2::Teleporter, f);
        WriteU32((uint32_t)pos.x, f); WriteU32((uint32_t)pos.y, f);
        WriteU32((uint32_t)tp->partner_entity_id, f);
        WriteColor(tp->color, f);
    } else if (ca->color_changer_arr.Get(entity_id)) {
        ColorChanger* cc = ca->color_changer_arr.Get(entity_id);
        bool movable = (ca->grid_mover_arr.Get(entity_id) != nullptr);
        WriteU32((uint32_t)EntityTypeV2::ColorChanger, f);
        WriteU32((uint32_t)pos.x, f); WriteU32((uint32_t)pos.y, f);
        WriteColor(cc->main_color, f);
        WriteU32((uint32_t)cc->mode, f);
        WriteU32(movable ? 1u : 0u, f);
    } else if (ca->endgoal_arr.Get(entity_id)) {
        WriteU32((uint32_t)EntityTypeV2::Endgoal, f);
        WriteU32((uint32_t)pos.x, f); WriteU32((uint32_t)pos.y, f);
    } else if (ca->grid_mover_arr.Get(entity_id)) {
        ColorTag* ct = ca->color_tag_arr.Get(entity_id);
        WriteU32((uint32_t)EntityTypeV2::PushBlock, f);
        WriteU32((uint32_t)pos.x, f); WriteU32((uint32_t)pos.y, f);
        WriteColor(ct ? ct->color : Color{ 255, 255, 255, 255 }, f);
    } else {
        WriteU32((uint32_t)EntityTypeV2::StaticBlock, f);
        WriteU32((uint32_t)pos.x, f); WriteU32((uint32_t)pos.y, f);
    }
}

void SaveLevelState(
    const char*      level_name,
    Tilemap*         tilemap,
    int              num_floor_tile_types,
    ComponentArrays* ca,
    int              num_entities
) {
    char filepath[256]{};
    snprintf(filepath, sizeof(filepath), "%s/%s.%s", level_directory, level_name, level_filetype_extension);

    FILE* f = fopen(filepath, "wb");
    if (!f) { printf("SaveLevelState: cannot open %s\n", filepath); return; }

    WriteU32((uint32_t)tilemap->width,          f);
    WriteU32((uint32_t)tilemap->height,         f);
    WriteU32((uint32_t)num_floor_tile_types,    f);
    WriteU32((uint32_t)num_entities,            f);

    for (int y = 0; y < tilemap->height; ++y)
        for (int x = 0; x < tilemap->width; ++x) {
            uint16_t tile = (uint16_t)tilemap->map[y * tilemap->width + x];
            fwrite(&tile, sizeof(uint16_t), 1, f);
        }

    for (int e = 0; e < num_entities; ++e)
        WriteEntityRecord(e, ca, f);

    fclose(f);
    printf("SaveLevelState: saved %s (%d entities)\n", filepath, num_entities);
}


// ============================================================
// Load
// ============================================================

// Reads one entity record, inserts components into ca, updates entity_map.
// Returns the entity_id used (== current *num_entities value before increment).
static void ReadEntityRecord(
    ComponentArrays* ca,
    EntityMap*       entity_map,
    int*             num_entities,  // incremented per entity loaded
    int*             player_ids,
    int*             num_players,
    FILE*            f
) {
    EntityTypeV2 type = (EntityTypeV2)ReadU32(f);
    int          x    = (int)ReadU32(f);
    int          y    = (int)ReadU32(f);

    int id = (*num_entities)++;

    switch (type) {
        case EntityTypeV2::Player: {
            Color     color       = ReadColor(f);
            Direction orientation = (Direction)ReadU32(f);
            PlayerInit(id, ca, { x, y }, orientation, color);
            entity_map->SetID(x, y, (int)GridLayer::EntityLayer, id);
            player_ids[(*num_players)++] = id;
            break;
        }
        case EntityTypeV2::PushBlock: {
            Color color = ReadColor(f);
            PushblockInit(id, ca, { x, y }, color);
            entity_map->SetID(x, y, (int)GridLayer::EntityLayer, id);
            break;
        }
        case EntityTypeV2::StaticBlock: {
            StaticBlockInit(id, ca, { x, y });
            entity_map->SetID(x, y, (int)GridLayer::EntityLayer, id);
            break;
        }
        case EntityTypeV2::Emitter: {
            Color     color = ReadColor(f);
            Direction dir   = (Direction)ReadU32(f);
            EmitterInit(id, ca, { x, y }, color, dir);
            entity_map->SetID(x, y, (int)GridLayer::EntityLayer, id);
            break;
        }
        case EntityTypeV2::Receiver: {
            Color   accepted = ReadColor(f);
            int32_t channels[MAX_CONNECTIONS];
            ReadChannels(channels, f);
            ReceiverInit(id, ca, { x, y }, accepted, channels);
            entity_map->SetID(x, y, (int)GridLayer::EntityLayer, id);
            break;
        }
        case EntityTypeV2::Door: {
            bool    open_by_default = ReadU32(f) != 0;
            int32_t channels[MAX_CONNECTIONS];
            ReadChannels(channels, f);
            DoorInit(id, ca, { x, y }, open_by_default, channels);
            entity_map->SetID(x, y, (int)GridLayer::GroundLayer, id);
            break;
        }
        case EntityTypeV2::Endgoal: {
            EndgoalInit(id, ca, { x, y });
            entity_map->SetID(x, y, (int)GridLayer::GroundLayer, id);
            break;
        }
        case EntityTypeV2::Button: {
            int32_t channels[MAX_CONNECTIONS];
            ReadChannels(channels, f);
            ButtonInit(id, ca, { x, y }, channels);
            entity_map->SetID(x, y, (int)GridLayer::GroundLayer, id);
            break;
        }
        case EntityTypeV2::Teleporter: {
            int   partner_id = (int)ReadU32(f);
            Color color      = ReadColor(f);
            TeleporterInit(id, ca, { x, y }, color, partner_id);
            entity_map->SetID(x, y, (int)GridLayer::GroundLayer, id);
            break;
        }
        case EntityTypeV2::ColorChanger: {
            Color          color   = ReadColor(f);
            ColorBlendMode mode    = (ColorBlendMode)ReadU32(f);
            bool           movable = ReadU32(f) != 0;
            ColorChangerInit(id, ca, { x, y }, color, mode, movable);
            GridLayer layer = movable ? GridLayer::EntityLayer : GridLayer::GroundLayer;
            entity_map->SetID(x, y, (int)layer, id);
            break;
        }
    }
}

LevelStateInfo ReadLevelState(
    const char*      level_name,
    Tilemap*         tilemap,
    ComponentArrays* ca,
    EntityMap*       entity_map,
    int*             player_ids,
    int*             num_players
) {
    char filepath[256]{};
    snprintf(filepath, sizeof(filepath), "%s/%s.%s", level_directory, level_name, level_filetype_extension);

    FILE* f = fopen(filepath, "rb");
    if (!f) { printf("ReadLevelState: cannot open %s\n", filepath); return { -1, -1 }; }

    int w            = (int)ReadU32(f);
    int h            = (int)ReadU32(f);
    int num_floor    = (int)ReadU32(f);
    int num_entities = (int)ReadU32(f);

    // (Re)allocate tilemap
    tilemap->width  = w;
    tilemap->height = h;
    if (!tilemap->map)
        tilemap->map = (int*)calloc(w * h, sizeof(int));
    else
        tilemap->map = (int*)realloc(tilemap->map, w * h * sizeof(int));

    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) {
            uint16_t tile = 0;
            fread(&tile, sizeof(tile), 1, f);
            tilemap->map[y * w + x] = tile;
        }

    // (Re)allocate entity_map
    entity_map->width  = w;
    entity_map->height = h;
    entity_map->depth  = 2;
    int map_size = w * h * 2;
    if (!entity_map->map)
        entity_map->map = (int*)malloc(map_size * sizeof(int));
    else
        entity_map->map = (int*)realloc(entity_map->map, map_size * sizeof(int));
    for (int i = 0; i < map_size; ++i) entity_map->map[i] = -1;

    // Reset players list
    *num_players = 0;
    for (int i = 0; i < 256; ++i) player_ids[i] = -1;

    // Clear component arrays and reload
    ca->Clear();

    int loaded = 0;
    for (int e = 0; e < num_entities; ++e)
        ReadEntityRecord(ca, entity_map, &loaded, player_ids, num_players, f);

    fclose(f);
    printf("ReadLevelState: loaded %s (%d entities, %d players)\n", filepath, loaded, *num_players);
    return { num_floor, loaded };
}
