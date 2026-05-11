#pragma once
#include "../Engine/skymath.h"
#include "../Engine/data_structs/sparse_set.h"
#include "tilemap.h"


// ============================================================
// SECTION: Spatial maps
// ============================================================

struct EntityMap {
    int* map;  // width x height x depth, indexed [y*width + x]*depth + z
    int  width;
    int  height;
    int  depth; // always 2: z=0 ground layer, z=1 entity layer

    int  GetID(int x, int y, int z) { return map[(y * width + x) * depth + z]; }
    void SetID(int x, int y, int z, int id) { map[(y * width + x) * depth + z] = id; }
};

struct EmissionTile {
    bool active;
    enum ORIENTATION_ENUM { HORIZONTAL, VERTICAL, CROSSED } orientation;
    Color color;
    Color vertical_color;
    Color horizontal_color;
};
struct EmissionMap {
    EmissionTile* map;
    int width;
    int height;

    EmissionTile GetEmissionTile(int x, int y)                    { return map[y * width + x]; }
    void         SetEmissionTile(int x, int y, EmissionTile tile) { map[y * width + x] = tile; }
};


// ============================================================
// SECTION: Component definitions
// ============================================================

enum class Direction      : uint8_t { Up, Right, Down, Left, Neutral };
enum class ColorBlendMode : uint8_t { Blended, Additive, Subtractive };
enum class GridLayer      : uint8_t { GroundLayer, EntityLayer };

#define MOVE_SPEED 0.2f  // seconds per block

struct GridPosition {
    Vector2Int position;
    Vector2Int prev_position; // cached for lerp, updated on move commit
    GridLayer  layer;
};

struct GridPlayerControlled {
    Direction orientation = Direction::Up;
    Color     color       = { 255, 255, 255, 255 };
};

struct GridMover {
    float move_timer = 0.0f;
    bool  moving     = false;
};

struct RenderTransform {
    Transform transform; // visual/lerped position, updated by EntityUpdateMover
};

struct LaserEmitter {
    Color     color;
    Direction dir = Direction::Up;
};

enum class LaserSurfaceMode : uint8_t { Absorb, PassThrough, Reflect, Refract };
struct LaserSurface {
    LaserSurfaceMode mode = LaserSurfaceMode::Absorb;
};

struct LaserReceiver {
    Color accepted_color;
    bool  received = false;  // true if any laser hit this frame
    bool  accepted = false;  // true if received color matches accepted_color
    Color incoming_color { 0, 0, 0, 0 };
};

#define MAX_CONNECTIONS 10
struct SignalChannel {
    int32_t channels[MAX_CONNECTIONS];
};

struct Door {
    bool is_open        = false;
    bool open_by_default = false;
};

struct Endgoal { };

struct Button {
    bool is_pressed = false;
};

struct Teleporter {
    int   partner_entity_id = -1; // entity ID of the connected teleporter
    Color color;
};

struct ColorChanger {
    Color          main_color;
    ColorBlendMode mode                  = ColorBlendMode::Blended;
    Color          vertical_input_color   { 0, 0, 0, 0 }; // frame-transient: cleared after each render
    Color          horizontal_input_color { 0, 0, 0, 0 };
};


// ============================================================
// SECTION: Entity handle
// ============================================================

struct Entity {
    uint32_t id         = 0;
    uint32_t generation = 0;
};


// ============================================================
// SECTION: ComponentArrays
// ============================================================

struct ComponentArrays {
    SparseSet<GridPosition>         grid_position_arr;
    SparseSet<GridPlayerControlled> grid_player_controlled_arr;
    SparseSet<GridMover>            grid_mover_arr;
    SparseSet<RenderTransform>      render_transform_arr;

    SparseSet<LaserEmitter>         laser_emitter_arr;
    SparseSet<LaserSurface>         laser_surface_arr;
    SparseSet<LaserReceiver>        laser_receiver_arr;

    SparseSet<SignalChannel>        signal_channel_arr;

    SparseSet<Door>                 door_arr;
    SparseSet<Endgoal>              endgoal_arr;
    SparseSet<Button>               button_arr;
    SparseSet<Teleporter>           teleporter_arr;
    SparseSet<ColorChanger>         color_changer_arr;

    void Init(int initial_capacity) {
        grid_position_arr.Init(initial_capacity);
        grid_player_controlled_arr.Init(initial_capacity);
        grid_mover_arr.Init(initial_capacity);
        render_transform_arr.Init(initial_capacity);
        laser_emitter_arr.Init(initial_capacity);
        laser_surface_arr.Init(initial_capacity);
        laser_receiver_arr.Init(initial_capacity);
        signal_channel_arr.Init(initial_capacity);
        door_arr.Init(initial_capacity);
        endgoal_arr.Init(initial_capacity);
        button_arr.Init(initial_capacity);
        teleporter_arr.Init(initial_capacity);
        color_changer_arr.Init(initial_capacity);
    }

    void Clear() {
        grid_position_arr.Clear();
        grid_player_controlled_arr.Clear();
        grid_mover_arr.Clear();
        render_transform_arr.Clear();
        laser_emitter_arr.Clear();
        laser_surface_arr.Clear();
        laser_receiver_arr.Clear();
        signal_channel_arr.Clear();
        door_arr.Clear();
        endgoal_arr.Clear();
        button_arr.Clear();
        teleporter_arr.Clear();
        color_changer_arr.Clear();
    }
};


// ============================================================
// SECTION: Archetype init helpers
// ============================================================

static inline SignalChannel MakeSignalChannel(int32_t src[MAX_CONNECTIONS]) {
    SignalChannel sc{};
    for (int i = 0; i < MAX_CONNECTIONS; ++i) sc.channels[i] = src ? src[i] : -1;
    return sc;
}

static inline RenderTransform MakeRenderTransform(Vector2Int pos, GridLayer layer) {
    RenderTransform rt{};
    rt.transform.position = { (float)pos.x, (float)pos.y, (layer == GridLayer::EntityLayer) ? 1.0f : 0.0f };
    rt.transform.rotation = { 0.f, 0.f, 0.f };
    rt.transform.scale    = { 1.f, 1.f, 1.f };
    return rt;
}

void PlayerInit(int id, ComponentArrays* ca, Vector2Int pos, Direction orientation, Color color) {
    ca->grid_position_arr.Insert(id, GridPosition{ pos, pos, GridLayer::EntityLayer });
    ca->grid_player_controlled_arr.Insert(id, GridPlayerControlled{ orientation, color });
    ca->grid_mover_arr.Insert(id, GridMover{});
    ca->render_transform_arr.Insert(id, MakeRenderTransform(pos, GridLayer::EntityLayer));
    ca->laser_surface_arr.Insert(id, LaserSurface{ LaserSurfaceMode::Absorb });
}

void PushblockInit(int id, ComponentArrays* ca, Vector2Int pos) {
    ca->grid_position_arr.Insert(id, GridPosition{ pos, pos, GridLayer::EntityLayer });
    ca->grid_mover_arr.Insert(id, GridMover{});
    ca->render_transform_arr.Insert(id, MakeRenderTransform(pos, GridLayer::EntityLayer));
    ca->laser_surface_arr.Insert(id, LaserSurface{ LaserSurfaceMode::Absorb });
}

void StaticBlockInit(int id, ComponentArrays* ca, Vector2Int pos) {
    ca->grid_position_arr.Insert(id, GridPosition{ pos, pos, GridLayer::EntityLayer });
    ca->render_transform_arr.Insert(id, MakeRenderTransform(pos, GridLayer::EntityLayer));
    ca->laser_surface_arr.Insert(id, LaserSurface{ LaserSurfaceMode::Absorb });
}

void EmitterInit(int id, ComponentArrays* ca, Vector2Int pos, Color color, Direction dir) {
    ca->grid_position_arr.Insert(id, GridPosition{ pos, pos, GridLayer::EntityLayer });
    ca->grid_mover_arr.Insert(id, GridMover{});
    ca->render_transform_arr.Insert(id, MakeRenderTransform(pos, GridLayer::EntityLayer));
    ca->laser_emitter_arr.Insert(id, LaserEmitter{ color, dir });
    ca->laser_surface_arr.Insert(id, LaserSurface{ LaserSurfaceMode::Absorb });
}

void ReceiverInit(int id, ComponentArrays* ca, Vector2Int pos, Color accepted_color, int32_t channels[MAX_CONNECTIONS]) {
    ca->grid_position_arr.Insert(id, GridPosition{ pos, pos, GridLayer::EntityLayer });
    ca->grid_mover_arr.Insert(id, GridMover{});
    ca->render_transform_arr.Insert(id, MakeRenderTransform(pos, GridLayer::EntityLayer));
    ca->laser_receiver_arr.Insert(id, LaserReceiver{ accepted_color });
    ca->laser_surface_arr.Insert(id, LaserSurface{ LaserSurfaceMode::Absorb });
    ca->signal_channel_arr.Insert(id, MakeSignalChannel(channels));
}

void DoorInit(int id, ComponentArrays* ca, Vector2Int pos, bool open_by_default, int32_t channels[MAX_CONNECTIONS]) {
    ca->grid_position_arr.Insert(id, GridPosition{ pos, pos, GridLayer::GroundLayer });
    ca->render_transform_arr.Insert(id, MakeRenderTransform(pos, GridLayer::GroundLayer));
    ca->door_arr.Insert(id, Door{ open_by_default, open_by_default });
    ca->laser_surface_arr.Insert(id, LaserSurface{ open_by_default ? LaserSurfaceMode::PassThrough : LaserSurfaceMode::Absorb });
    ca->signal_channel_arr.Insert(id, MakeSignalChannel(channels));
}

void EndgoalInit(int id, ComponentArrays* ca, Vector2Int pos) {
    ca->grid_position_arr.Insert(id, GridPosition{ pos, pos, GridLayer::GroundLayer });
    ca->render_transform_arr.Insert(id, MakeRenderTransform(pos, GridLayer::GroundLayer));
    ca->endgoal_arr.Insert(id, Endgoal{});
    ca->laser_surface_arr.Insert(id, LaserSurface{ LaserSurfaceMode::PassThrough });
}

void ButtonInit(int id, ComponentArrays* ca, Vector2Int pos, int32_t channels[MAX_CONNECTIONS]) {
    ca->grid_position_arr.Insert(id, GridPosition{ pos, pos, GridLayer::GroundLayer });
    ca->render_transform_arr.Insert(id, MakeRenderTransform(pos, GridLayer::GroundLayer));
    ca->button_arr.Insert(id, Button{});
    ca->laser_surface_arr.Insert(id, LaserSurface{ LaserSurfaceMode::PassThrough });
    ca->signal_channel_arr.Insert(id, MakeSignalChannel(channels));
}

void TeleporterInit(int id, ComponentArrays* ca, Vector2Int pos, Color color, int partner_entity_id) {
    ca->grid_position_arr.Insert(id, GridPosition{ pos, pos, GridLayer::GroundLayer });
    ca->render_transform_arr.Insert(id, MakeRenderTransform(pos, GridLayer::GroundLayer));
    ca->teleporter_arr.Insert(id, Teleporter{ partner_entity_id, color });
    ca->laser_surface_arr.Insert(id, LaserSurface{ LaserSurfaceMode::PassThrough });
}

void ColorChangerInit(int id, ComponentArrays* ca, Vector2Int pos, Color color, ColorBlendMode mode, bool movable) {
    GridLayer layer = movable ? GridLayer::EntityLayer : GridLayer::GroundLayer;
    ca->grid_position_arr.Insert(id, GridPosition{ pos, pos, layer });
    if (movable) ca->grid_mover_arr.Insert(id, GridMover{});
    ca->render_transform_arr.Insert(id, MakeRenderTransform(pos, layer));
    ca->color_changer_arr.Insert(id, ColorChanger{ color, mode });
    ca->laser_surface_arr.Insert(id, LaserSurface{ LaserSurfaceMode::PassThrough });
}


// ============================================================
// SECTION: Color helpers
// ============================================================

static inline Color BlendColor(Color a, Color b) {
    if (a.a == 0) return b;
    if (b.a == 0) return a;
    return Color{
        uint8_t((a.r + b.r) / 2),
        uint8_t((a.g + b.g) / 2),
        uint8_t((a.b + b.b) / 2),
        uint8_t((a.a + b.a) / 2)
    };
}

static inline Color AddColor(Color a, Color b) {
    if (b.a == 0) return a;
    if (a.a == 0) return b;
    return a + b;
}

static inline Color SubtractColor(Color a, Color b) {
    return Color{
        (uint8_t)IntClamp(a.r - b.r, 0, 255),
        (uint8_t)IntClamp(a.g - b.g, 0, 255),
        (uint8_t)IntClamp(a.b - b.b, 0, 255),
        (uint8_t)IntClamp(a.a - b.a, 0, 255)
    };
}


// ============================================================
// SECTION: Movement system
// ============================================================

bool EntityMove(
    int        entity_id,
    Vector2Int direction,
    Tilemap    tilemap,
    EntityMap& entity_map,
    ComponentArrays* ca,
    int move_weight = 1
) {
    if (move_weight < 0) return false;

    GridPosition* gp = ca->grid_position_arr.Get(entity_id);
    GridMover*    gm = ca->grid_mover_arr.Get(entity_id);
    if (!gp || !gm) return false; // entity must be movable

    Vector2Int new_pos = gp->position + direction;

    // Bounds check
    if (new_pos.x < 0 || new_pos.y < 0 || new_pos.x >= tilemap.width || new_pos.y >= tilemap.height) return false;
    if (TestTileCollide(tilemap, new_pos)) return false;

    // Check floor layer at new_pos: teleporter and door
    {
        int floor_id = entity_map.GetID(new_pos.x, new_pos.y, 0);
        if (floor_id >= 0) {
            Teleporter* tp = ca->teleporter_arr.Get(floor_id);
            if (tp && tp->partner_entity_id >= 0) {
                GridPosition* partner_gp = ca->grid_position_arr.Get(tp->partner_entity_id);
                if (partner_gp) {
                    Vector2Int teleport_dest = partner_gp->position + direction;
                    if (teleport_dest.x >= 0 && teleport_dest.y >= 0 &&
                        teleport_dest.x < tilemap.width && teleport_dest.y < tilemap.height &&
                        !TestTileCollide(tilemap, teleport_dest)) {
                        new_pos = teleport_dest;
                    }
                }
            }
        }

        int new_floor_id = entity_map.GetID(new_pos.x, new_pos.y, 0);
        if (new_floor_id >= 0) {
            Door* door = ca->door_arr.Get(new_floor_id);
            if (door && !door->is_open) return false;
        }
    }

    // Check entity layer for a blocking entity; try to push it
    int blocking_id = entity_map.GetID(new_pos.x, new_pos.y, (int)gp->layer);
    if (blocking_id >= 0) {
        bool pushed = EntityMove(blocking_id, direction, tilemap, entity_map, ca, move_weight - 1);
        if (!pushed) return false;
    }

    // Commit the move
    entity_map.SetID(gp->position.x, gp->position.y, (int)gp->layer, -1);
    gp->prev_position = gp->position;
    gp->position      = new_pos;
    entity_map.SetID(new_pos.x, new_pos.y, (int)gp->layer, entity_id);
    gm->moving     = true;
    gm->move_timer = 0.0f;

    return true;
}


// ============================================================
// SECTION: Per-frame update systems
// ============================================================

void EntityUpdateMover(int entity_id, ComponentArrays* ca, float dt) {
    GridPosition*    gp = ca->grid_position_arr.Get(entity_id);
    GridMover*       gm = ca->grid_mover_arr.Get(entity_id);
    RenderTransform* rt = ca->render_transform_arr.Get(entity_id);
    if (!gp || !gm || !rt || !gm->moving) return;

    gm->move_timer += dt;
    float t = gm->move_timer / (MOVE_SPEED * 1000.0f);

    rt->transform.position.x = Lerp((float)gp->prev_position.x, (float)gp->position.x, t);
    rt->transform.position.y = Lerp((float)gp->prev_position.y, (float)gp->position.y, t);

    if (t >= 1.0f) {
        rt->transform.position.x = (float)gp->position.x;
        rt->transform.position.y = (float)gp->position.y;
        gm->move_timer    = 0.0f;
        gm->moving        = false;
        gp->prev_position = gp->position;
    }
}

void EntityUpdateButton(int entity_id, ComponentArrays* ca, EntityMap& entity_map) {
    GridPosition* gp  = ca->grid_position_arr.Get(entity_id);
    Button*       btn = ca->button_arr.Get(entity_id);
    if (!gp || !btn) return;

    int top_id = entity_map.GetID(gp->position.x, gp->position.y, (int)GridLayer::EntityLayer);
    btn->is_pressed = (top_id >= 0);
}

void EntityUpdateReceiver(int entity_id, ComponentArrays* ca) {
    LaserReceiver* lr = ca->laser_receiver_arr.Get(entity_id);
    if (!lr) return;

    if (lr->received) {
        lr->accepted = CompareColorWithTolerance(lr->incoming_color, lr->accepted_color, 2);
    } else {
        lr->incoming_color = { 0, 0, 0, 0 };
        lr->accepted       = false;
    }
}

void EntityUpdateDoor(int entity_id, ComponentArrays* ca, EntityMap& entity_map, int num_entities) {
    GridPosition* gp   = ca->grid_position_arr.Get(entity_id);
    Door*         door = ca->door_arr.Get(entity_id);
    if (!gp || !door) return;

    SignalChannel* door_ch = ca->signal_channel_arr.Get(entity_id);

    bool activated = false;
    if (door_ch) {
        for (int i = 0; i < num_entities && !activated; ++i) {
            SignalChannel* act_ch = ca->signal_channel_arr.Get(i);
            if (!act_ch) continue;

            LaserReceiver* lr  = ca->laser_receiver_arr.Get(i);
            Button*        btn = ca->button_arr.Get(i);
            bool is_active = (lr && lr->accepted) || (btn && btn->is_pressed);
            if (!is_active) continue;

            for (int d = 0; d < MAX_CONNECTIONS && !activated; ++d) {
                if (door_ch->channels[d] < 0) continue;
                for (int a = 0; a < MAX_CONNECTIONS && !activated; ++a) {
                    if (act_ch->channels[a] < 0) continue;
                    if (door_ch->channels[d] == act_ch->channels[a]) activated = true;
                }
            }
        }
    }

    door->is_open = door->open_by_default ? !activated : activated;

    // Keep entity_map layer 1 in sync so closed doors block movement
    int top_id           = entity_map.GetID(gp->position.x, gp->position.y, (int)GridLayer::EntityLayer);
    bool occupied_by_other = top_id >= 0 && top_id != entity_id;

    if (!door->is_open && !occupied_by_other) {
        entity_map.SetID(gp->position.x, gp->position.y, (int)GridLayer::EntityLayer, entity_id);
    } else if (door->is_open && top_id == entity_id) {
        entity_map.SetID(gp->position.x, gp->position.y, (int)GridLayer::EntityLayer, -1);
    }

    // Sync LaserSurface passability with open state
    LaserSurface* ls = ca->laser_surface_arr.Get(entity_id);
    if (ls) ls->mode = door->is_open ? LaserSurfaceMode::PassThrough : LaserSurfaceMode::Absorb;
}


// ============================================================
// SECTION: Laser / emission system
// ============================================================

void EntityUpdateColorChanger(int entity_id, ComponentArrays* ca, EntityMap& entity_map,
                               EmissionMap& emission_map, Tilemap tilemap, Vector2Int direction);

void UpdateEmit(
    int        emitter_id,
    Vector2Int direction,
    Vector2Int pos,
    Color      color,
    Tilemap    tilemap,
    EntityMap& entity_map,
    EmissionMap& emission_map,
    ComponentArrays* ca
) {
    Vector2Int new_pos = pos + direction;
    if (new_pos.x < 0 || new_pos.y < 0 || new_pos.x >= tilemap.width || new_pos.y >= tilemap.height) return;
    if (TestTileCollide(tilemap, new_pos)) return;

    // Hit entity-layer entity
    int entity_id = entity_map.GetID(new_pos.x, new_pos.y, (int)GridLayer::EntityLayer);
    if (entity_id >= 0) {
        LaserReceiver* lr = ca->laser_receiver_arr.Get(entity_id);
        if (lr) {
            if (lr->received) lr->incoming_color = AddColor(color, lr->incoming_color);
            else { lr->incoming_color = color; lr->received = true; }
        }

        ColorChanger* cc = ca->color_changer_arr.Get(entity_id);
        if (cc) {
            EntityUpdateColorChanger(entity_id, ca, entity_map, emission_map, tilemap, direction);
        }

        LaserSurface* ls = ca->laser_surface_arr.Get(entity_id);
        if (!ls || ls->mode == LaserSurfaceMode::Absorb) return;
        // PassThrough: fall through and continue the beam
    }

    // Closed door blocks the laser
    int floor_id = entity_map.GetID(new_pos.x, new_pos.y, (int)GridLayer::GroundLayer);
    if (floor_id >= 0) {
        Door* door = ca->door_arr.Get(floor_id);
        if (door && !door->is_open) return;
    }

    // Write emission tile
    EmissionTile tile        = emission_map.GetEmissionTile(new_pos.x, new_pos.y);
    auto         orientation = (direction.y != 0) ? EmissionTile::VERTICAL : EmissionTile::HORIZONTAL;

    if (tile.active) {
        if (tile.orientation != orientation) {
            if (tile.orientation == EmissionTile::VERTICAL) {
                tile.vertical_color   = tile.color;
                tile.horizontal_color = color;
            } else if (tile.orientation == EmissionTile::HORIZONTAL) {
                tile.vertical_color   = color;
                tile.horizontal_color = tile.color;
            } else { // already CROSSED
                if (orientation == EmissionTile::VERTICAL)
                    tile.vertical_color   = AddColor(tile.vertical_color,   color);
                else
                    tile.horizontal_color = AddColor(tile.horizontal_color, color);
            }
            tile.orientation = EmissionTile::CROSSED;
        }
        tile.color = AddColor(tile.color, color);
    } else {
        tile = { true, orientation, color, { 0,0,0,0 }, { 0,0,0,0 } };
        if (orientation == EmissionTile::VERTICAL)   tile.vertical_color   = color;
        else                                          tile.horizontal_color = color;
    }
    emission_map.SetEmissionTile(new_pos.x, new_pos.y, tile);

    UpdateEmit(emitter_id, direction, new_pos, color, tilemap, entity_map, emission_map, ca);
}

void EntityUpdateEmit(int entity_id, ComponentArrays* ca, Tilemap tilemap, EntityMap& entity_map, EmissionMap& emission_map) {
    LaserEmitter* le = ca->laser_emitter_arr.Get(entity_id);
    GridPosition* gp = ca->grid_position_arr.Get(entity_id);
    GridMover*    gm = ca->grid_mover_arr.Get(entity_id);
    if (!le || !gp) return;
    if (gm && gm->moving) return; // don't emit while moving

    Vector2Int dir = { 0, 0 };
    switch (le->dir) {
        case Direction::Up:    dir.y =  1; break;
        case Direction::Down:  dir.y = -1; break;
        case Direction::Right: dir.x =  1; break;
        case Direction::Left:  dir.x = -1; break;
        default: break;
    }
    UpdateEmit(entity_id, dir, gp->position, le->color, tilemap, entity_map, emission_map, ca);
}

void EntityUpdateColorChanger(int entity_id, ComponentArrays* ca, EntityMap& entity_map,
                               EmissionMap& emission_map, Tilemap tilemap, Vector2Int direction) {
    GridPosition* gp = ca->grid_position_arr.Get(entity_id);
    ColorChanger* cc = ca->color_changer_arr.Get(entity_id);
    if (!gp || !cc) return;

    bool       is_vertical = (direction.y != 0);
    Vector2Int prev_pos    = gp->position - direction;

    if (prev_pos.x < 0 || prev_pos.y < 0 || prev_pos.x >= emission_map.width || prev_pos.y >= emission_map.height) return;

    EmissionTile::ORIENTATION_ENUM not_orientation = is_vertical ? EmissionTile::HORIZONTAL : EmissionTile::VERTICAL;
    EmissionTile prev_tile   = emission_map.GetEmissionTile(prev_pos.x, prev_pos.y);
    int          prev_ent_id = entity_map.GetID(prev_pos.x, prev_pos.y, (int)GridLayer::EntityLayer);

    Color input_color { 0, 0, 0, 0 };
    bool  has_input = false;

    if (prev_tile.active && prev_tile.orientation != not_orientation) {
        input_color = is_vertical ? prev_tile.vertical_color : prev_tile.horizontal_color;
        has_input   = true;
    } else if (prev_ent_id >= 0) {
        LaserEmitter* le  = ca->laser_emitter_arr.Get(prev_ent_id);
        ColorChanger* pcc = ca->color_changer_arr.Get(prev_ent_id);
        if (le) {
            input_color = le->color;
            has_input   = true;
        } else if (pcc) {
            input_color = is_vertical ? pcc->vertical_input_color : pcc->horizontal_input_color;
            has_input   = input_color.a > 0;
        }
    }

    if (!has_input) return;

    Color out_color;
    switch (cc->mode) {
        case ColorBlendMode::Additive:    out_color = AddColor(input_color, cc->main_color);      break;
        case ColorBlendMode::Subtractive: out_color = SubtractColor(input_color, cc->main_color); break;
        default:                          out_color = BlendColor(input_color, cc->main_color);    break;
    }

    if (is_vertical) cc->vertical_input_color   = out_color;
    else             cc->horizontal_input_color  = out_color;

    UpdateEmit(entity_id, direction, gp->position, out_color, tilemap, entity_map, emission_map, ca);
}


// ============================================================
// SECTION: Frame-state clear (call once per frame, before UpdateEmit)
// ============================================================

void ClearFrameState(ComponentArrays* ca) {
    for (auto& lr : ca->laser_receiver_arr.dense) {
        lr.received       = false;
        lr.incoming_color = { 0, 0, 0, 0 };
    }
    for (auto& cc : ca->color_changer_arr.dense) {
        cc.vertical_input_color   = { 0, 0, 0, 0 };
        cc.horizontal_input_color = { 0, 0, 0, 0 };
    }
}
