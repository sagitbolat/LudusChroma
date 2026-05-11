// migrate_levels.cpp
// Converts v1 .level files to v2 format.
//
// Usage:
//   migrate.exe <input_dir> <output_dir>
//
// Reads every level named in the game's level list from input_dir, writes v2
// files to output_dir (which must already exist).
//
// Compile (standalone, no engine deps):
//   g++ -o migrate.exe migrate_levels.cpp

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// ============================================================
// V1 constants (from Color_game/entity.h and level_loader.h)
// ============================================================
#define V1_MAX_CONNECTED 32

#define V1_PLAYER        0
#define V1_PUSH_BLOCK    1
#define V1_STATIC_BLOCK  2
#define V1_EMITTER       3
#define V1_RECEIVER      4
#define V1_DOOR          5
#define V1_BUTTON        6
#define V1_ENDGOAL       7
#define V1_TELEPORTER    8
#define V1_COLOR_CHANGER 9
#define V1_COLOR_GATE    10

// ============================================================
// V2 constants (from Color_game_v2/level_loader.h)
// ============================================================
#define V2_MAX_CONNECTIONS 10

#define V2_PLAYER        0
#define V2_PUSH_BLOCK    1
#define V2_STATIC_BLOCK  2
#define V2_EMITTER       3
#define V2_RECEIVER      4
#define V2_DOOR          5
#define V2_ENDGOAL       6
#define V2_BUTTON        7
#define V2_TELEPORTER    8
#define V2_COLOR_CHANGER 9

// ============================================================
// V1 entity record (in-memory representation of 64 uint32 blob)
// ============================================================
struct V1Entity {
    int id;
    int active;
    int pos_x, pos_y;
    int entity_type;
    int main_color[4];
    int emitter_dir;
    int emitter_color[4];
    int receiver_accepted[4];
    int door_open_by_default;
    int door_num_connected;
    int door_connected_ids[V1_MAX_CONNECTED];
    int teleporter_active;
    int teleporter_color[4];
    int teleporter_connected_id;
    int cc_active;
    int cc_color[4];
    int cc_color_mode;
};

// ============================================================
// I/O helpers
// ============================================================
static uint32_t ReadU32(FILE* f) {
    uint32_t v = 0;
    fread(&v, sizeof(v), 1, f);
    return v;
}
static void WriteU32(uint32_t v, FILE* f) {
    fwrite(&v, sizeof(v), 1, f);
}
static void WriteColor4(int r, int g, int b, int a, FILE* f) {
    WriteU32((uint32_t)r, f);
    WriteU32((uint32_t)g, f);
    WriteU32((uint32_t)b, f);
    WriteU32((uint32_t)a, f);
}
static void WriteChannels(int32_t ch[V2_MAX_CONNECTIONS], FILE* f) {
    for (int i = 0; i < V2_MAX_CONNECTIONS; ++i)
        WriteU32((uint32_t)ch[i], f);
}

// ============================================================
// Read one v1 entity blob (exactly 64 uint32 = 256 bytes)
// ============================================================
static void ReadV1Entity(V1Entity* e, FILE* f) {
    e->id                     = (int)ReadU32(f);
    e->active                 = (int)ReadU32(f);
    e->pos_x                  = (int)ReadU32(f);
    e->pos_y                  = (int)ReadU32(f);
    e->entity_type            = (int)ReadU32(f);
    for (int i = 0; i < 4; ++i) e->main_color[i]      = (int)ReadU32(f);
    e->emitter_dir            = (int)ReadU32(f);
    for (int i = 0; i < 4; ++i) e->emitter_color[i]   = (int)ReadU32(f);
    for (int i = 0; i < 4; ++i) e->receiver_accepted[i] = (int)ReadU32(f);
    e->door_open_by_default   = (int)ReadU32(f);
    e->door_num_connected     = (int)ReadU32(f);
    for (int i = 0; i < V1_MAX_CONNECTED; ++i) e->door_connected_ids[i] = (int)ReadU32(f);
    e->teleporter_active      = (int)ReadU32(f);
    for (int i = 0; i < 4; ++i) e->teleporter_color[i] = (int)ReadU32(f);
    e->teleporter_connected_id = (int)ReadU32(f);
    e->cc_active              = (int)ReadU32(f);
    for (int i = 0; i < 4; ++i) e->cc_color[i]        = (int)ReadU32(f);
    e->cc_color_mode          = (int)ReadU32(f);
}

// ============================================================
// Migrate one level file
// ============================================================
static void MigrateLevel(const char* in_path, const char* out_path) {
    FILE* fin = fopen(in_path, "rb");
    if (!fin) {
        printf("  SKIP (not found): %s\n", in_path);
        return;
    }

    // --- Read v1 header ---
    uint32_t width         = ReadU32(fin);
    uint32_t height        = ReadU32(fin);
    uint32_t num_floor     = ReadU32(fin);
    uint32_t num_entities  = ReadU32(fin);

    // --- Read tilemap (uint16 per tile — same in v1 and v2) ---
    uint16_t* tilemap = (uint16_t*)malloc(width * height * sizeof(uint16_t));
    fread(tilemap, sizeof(uint16_t), width * height, fin);

    // --- Read all v1 entity blobs ---
    V1Entity* ents = (V1Entity*)malloc(num_entities * sizeof(V1Entity));
    for (uint32_t i = 0; i < num_entities; ++i)
        ReadV1Entity(&ents[i], fin);
    fclose(fin);

    // --- Build v1_id → v2_id mapping (inactive entities are skipped in v2) ---
    // v1_id is the entity index in the array (0..num_entities-1).
    int* id_map = (int*)malloc(num_entities * sizeof(int));
    for (uint32_t i = 0; i < num_entities; ++i) id_map[i] = -1;
    {
        int next_v2_id = 0;
        for (uint32_t i = 0; i < num_entities; ++i) {
            if (!ents[i].active) continue;
            int type = ents[i].entity_type;
            if (type == V1_COLOR_GATE) continue; // no v2 equivalent, skip
            id_map[i] = next_v2_id++;
        }
    }

    // --- Count active exportable entities ---
    int num_active = 0;
    for (uint32_t i = 0; i < num_entities; ++i) {
        if (ents[i].active && ents[i].entity_type != V1_COLOR_GATE) ++num_active;
    }

    // --- Write v2 file ---
    FILE* fout = fopen(out_path, "wb");
    if (!fout) {
        printf("  FAIL (cannot write): %s\n", out_path);
        free(tilemap); free(ents); free(id_map);
        return;
    }

    WriteU32(width,    fout);
    WriteU32(height,   fout);
    WriteU32(num_floor, fout);
    WriteU32((uint32_t)num_active, fout);

    fwrite(tilemap, sizeof(uint16_t), width * height, fout);

    for (uint32_t i = 0; i < num_entities; ++i) {
        V1Entity* e = &ents[i];
        if (!e->active) continue;

        int v1_type = e->entity_type;
        if (v1_type == V1_COLOR_GATE) continue;

        // Map v1 type to v2 type.
        // Note: v1 BUTTON=6,ENDGOAL=7 — v2 Endgoal=6,Button=7 (swapped!)
        uint32_t v2_type;
        switch (v1_type) {
            case V1_PLAYER:        v2_type = V2_PLAYER;        break;
            case V1_PUSH_BLOCK:    v2_type = V2_PUSH_BLOCK;    break;
            case V1_STATIC_BLOCK:  v2_type = V2_STATIC_BLOCK;  break;
            case V1_EMITTER:       v2_type = V2_EMITTER;       break;
            case V1_RECEIVER:      v2_type = V2_RECEIVER;      break;
            case V1_DOOR:          v2_type = V2_DOOR;          break;
            case V1_BUTTON:        v2_type = V2_BUTTON;        break;
            case V1_ENDGOAL:       v2_type = V2_ENDGOAL;       break;
            case V1_TELEPORTER:    v2_type = V2_TELEPORTER;    break;
            case V1_COLOR_CHANGER: v2_type = V2_COLOR_CHANGER; break;
            default: continue;
        }

        WriteU32(v2_type,          fout);
        WriteU32((uint32_t)e->pos_x, fout);
        WriteU32((uint32_t)e->pos_y, fout);

        switch (v2_type) {
            case V2_PLAYER: {
                // main_color is the suit color; orientation defaults to Up (0)
                WriteColor4(e->main_color[0], e->main_color[1], e->main_color[2], e->main_color[3], fout);
                WriteU32(0u, fout); // Direction::Up = 0
                break;
            }
            case V2_PUSH_BLOCK:
            case V2_STATIC_BLOCK:
            case V2_ENDGOAL:
                // No extra fields
                break;

            case V2_EMITTER: {
                // emission_color, then direction
                // Direction enum matches between v1 and v2: Up=0 Right=1 Down=2 Left=3
                WriteColor4(e->emitter_color[0], e->emitter_color[1], e->emitter_color[2], e->emitter_color[3], fout);
                WriteU32((uint32_t)e->emitter_dir, fout);
                break;
            }
            case V2_RECEIVER: {
                // accepted_color, then channels[]
                // Channel for this activator = its v1 entity ID (doors store v1 IDs too, so they match)
                WriteColor4(e->receiver_accepted[0], e->receiver_accepted[1], e->receiver_accepted[2], e->receiver_accepted[3], fout);
                int32_t ch[V2_MAX_CONNECTIONS];
                for (int c = 0; c < V2_MAX_CONNECTIONS; ++c) ch[c] = -1;
                ch[0] = e->id;
                WriteChannels(ch, fout);
                break;
            }
            case V2_DOOR: {
                // open_by_default, then channels[] = v1 connected activator IDs
                WriteU32((uint32_t)e->door_open_by_default, fout);
                int32_t ch[V2_MAX_CONNECTIONS];
                for (int c = 0; c < V2_MAX_CONNECTIONS; ++c) ch[c] = -1;
                int n = e->door_num_connected;
                if (n > V2_MAX_CONNECTIONS) {
                    printf("  WARNING: door at (%d,%d) has %d connections, capping at %d\n",
                           e->pos_x, e->pos_y, n, V2_MAX_CONNECTIONS);
                    n = V2_MAX_CONNECTIONS;
                }
                for (int c = 0; c < n; ++c) ch[c] = e->door_connected_ids[c];
                WriteChannels(ch, fout);
                break;
            }
            case V2_BUTTON: {
                // channels[] — channel = its v1 entity ID
                int32_t ch[V2_MAX_CONNECTIONS];
                for (int c = 0; c < V2_MAX_CONNECTIONS; ++c) ch[c] = -1;
                ch[0] = e->id;
                WriteChannels(ch, fout);
                break;
            }
            case V2_TELEPORTER: {
                // partner_entity_id (remapped), then color
                int partner_v1 = e->teleporter_connected_id;
                int partner_v2 = -1;
                if (partner_v1 >= 0 && partner_v1 < (int)num_entities)
                    partner_v2 = id_map[partner_v1];
                WriteU32((uint32_t)partner_v2, fout);
                WriteColor4(e->teleporter_color[0], e->teleporter_color[1], e->teleporter_color[2], e->teleporter_color[3], fout);
                break;
            }
            case V2_COLOR_CHANGER: {
                // main_color, mode, movable (always true in v1)
                WriteColor4(e->cc_color[0], e->cc_color[1], e->cc_color[2], e->cc_color[3], fout);
                WriteU32((uint32_t)e->cc_color_mode, fout);
                WriteU32(1u, fout); // movable = true
                break;
            }
        }
    }

    fclose(fout);
    free(tilemap); free(ents); free(id_map);
    printf("  OK  %-20s  (%d entities -> %d active)\n", out_path, num_entities, num_active);
}

// ============================================================
// Level list (matches game.cpp level_names[])
// ============================================================
static const char* level_names[] = {
    "0",    "1",    "2",    "3",    "4",
    "5_w",  "6_w",  "7_w",  "29",   "10_w",
    "11_w", "12_w", "13_w", "14_w", "15_w",
    "16_w", "24_w", "25_w", "26_w", "17_w",
    "18_w", "20_w", "27_w", "9_w",  "19_w",
    "23_w", "21_w", "8_w",  "28_w", "22_w2222",
    "32_22","33_w", "31_wh","34_l", "nd2",
    nullptr
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: migrate.exe <input_dir> <output_dir>\n");
        printf("  Migrates all game levels from v1 to v2 format.\n");
        return 1;
    }
    const char* in_dir  = argv[1];
    const char* out_dir = argv[2];

    char in_path[512], out_path[512];
    for (int i = 0; level_names[i]; ++i) {
        snprintf(in_path,  sizeof(in_path),  "%s/%s.level", in_dir,  level_names[i]);
        snprintf(out_path, sizeof(out_path), "%s/%s.level", out_dir, level_names[i]);
        MigrateLevel(in_path, out_path);
    }
    return 0;
}
