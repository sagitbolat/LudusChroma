# Color Game V2 — Codebase Reference

## Table of Contents

1. [Architecture Overview](#architecture-overview)
2. [Key Parameters](#key-parameters)
3. [Component System](#component-system)
4. [Adding a New Component](#adding-a-new-component)
5. [Animated Sprites](#animated-sprites)
6. [Level Format](#level-format)
7. [Color / Hidden Color System](#color--hidden-color-system)
8. [Rendering Pipeline](#rendering-pipeline)
9. [Movement & Undo](#movement--undo)
10. [Signal / Wiring System](#signal--wiring-system)
11. [Editor](#editor)
12. [Scene Management](#scene-management)

---

## Architecture Overview

The game is a **unity build**: `game.cpp` `#include`s all `.h` files as one translation unit. Include order matters for global variable visibility.

```
game.cpp
  tilemap.h            — Tilemap struct, tile collision
  entity.h             — All component structs, ComponentArrays, archetype inits, update systems, EntityMove
  entity_render.h      — EntityRender, EmissionRender, UV helpers, sprite index constants
  level_loader.h       — Level binary I/O (save/load), EntityTypeV2 enum
  game_editor.h        — ImGui editor panel
  game_render.h        — GameRender() draw passes, SetEntityZ, SetEntityIrisMask, DrawWires
  game_scenes.h        — TitleUpdate, Act2Update, GameUpdate (input, per-frame logic)
  game_color_switcher.h — Hidden color array, CheckHiddenColorSwitch, CommitHiddenColorSwitch, isHidden
```

**ECS**: Entities are plain integers. Component storage is `SparseSet<T>` (see `Engine/data_structs/sparse_set.h`). All component arrays live in a single `ComponentArrays comp_arrays` global.

**Spatial grid**: `EntityMap entity_map` is a flat `int[width * height * 2]` grid. Index = `(y*width + x)*2 + layer`. Layer 0 = `GroundLayer`, layer 1 = `EntityLayer`. A cell value of `-1` means empty.

---

## Key Parameters

### Player Speed
`entity.h:48`
```cpp
#define MOVE_SPEED 0.2f  // seconds per block
```
Controls how long the slide animation takes. Lower = faster.

### Level List
`game.cpp:57–66`
```cpp
const int NUM_LEVELS = 15;
int curr_level_index = 0;

char level_names[][64] = {
    "0", "1", ... "14"
};

float level_zoom[] = {
    14.f, 14.f, ...   // camera width in world units, one entry per level
};
```
Add/remove entries to both arrays when adding levels. `curr_level_index` starts at 0 and is incremented to 1 before the first `Start()` call, so the game begins at level 1 (file `1.level`). Level 0 is loaded in `Awake` for preloading purposes.

### Iris Animation Duration
`game.cpp:107`
```cpp
const float IRIS_DURATION = 400.f;  // milliseconds
```

### Door / Button Animation Duration
Set per-instance in `DoorInit` and `ButtonInit` inside `entity.h`.
- Door: `anim_duration = 400.f` ms (open/close)
- Button press: `anim_duration = 300.f` ms

To change globally, edit the defaults in those init functions.

### Camera Zoom
Per-level in the `level_zoom[]` array in `game.cpp`. Value is the camera width in world units; height is derived from the screen aspect ratio.

### Screen Resolution
`game.cpp` in `Init()`:
```cpp
*w = 1280;
*h = 720;
```

### Max Entities Per Level
`game.cpp:12`
```cpp
#define MAX_ENTITIES 256
```

---

## Component System

All components are defined in `entity.h`. Each is stored in its own `SparseSet<T>` inside `ComponentArrays`.

### Component Reference

| Component | Array | Description |
|---|---|---|
| `GridPosition` | `grid_position_arr` | Grid coords, prev coords (for lerp), layer |
| `GridPlayerControlled` | `grid_player_controlled_arr` | Orientation, color, upwards_direction |
| `GridMover` | `grid_mover_arr` | move_timer, moving flag — any entity that can slide |
| `RenderTransform` | `render_transform_arr` | Visual/lerped world position for drawing |
| `LaserEmitter` | `laser_emitter_arr` | Emitter color and direction |
| `LaserSurface` | `laser_surface_arr` | How the entity interacts with laser beams |
| `LaserReceiver` | `laser_receiver_arr` | Accepted color, received/accepted flags, incoming color |
| `SignalChannel` | `signal_channel_arr` | 10 channel IDs used for wiring activators to doors |
| `Door` | `door_arr` | Open/close state, animation state/timer |
| `Endgoal` | `endgoal_arr` | Level exit marker (empty struct) |
| `Button` | `button_arr` | Press state, animation state/timer |
| `Teleporter` | `teleporter_arr` | Partner entity ID, color |
| `ColorChanger` | `color_changer_arr` | Main color, blend mode, transient laser input colors |
| `ColorTag` | `color_tag_arr` | Entity's color for the hidden-color system |

### GridLayer Enum
```
GroundLayer = 0  — doors, endgoals, buttons, teleporters, ground color changers
EntityLayer = 1  — players, push blocks, static blocks, emitters, receivers, movable color changers
```

### Archetype Init Functions (entity.h)
Each entity type has a dedicated init that inserts the right set of components:

| Function | Components inserted |
|---|---|
| `PlayerInit` | GridPosition, GridPlayerControlled, GridMover, RenderTransform, LaserSurface, ColorTag |
| `PushblockInit` | GridPosition, GridMover, RenderTransform, LaserSurface, ColorTag |
| `StaticBlockInit` | GridPosition, RenderTransform, LaserSurface |
| `EmitterInit` | GridPosition, GridMover, RenderTransform, LaserEmitter, LaserSurface |
| `ReceiverInit` | GridPosition, GridMover, RenderTransform, LaserReceiver, LaserSurface, SignalChannel |
| `DoorInit` | GridPosition, RenderTransform, Door, LaserSurface, SignalChannel |
| `EndgoalInit` | GridPosition, RenderTransform, Endgoal, LaserSurface |
| `ButtonInit` | GridPosition, RenderTransform, Button, LaserSurface, SignalChannel |
| `TeleporterInit` | GridPosition, RenderTransform, Teleporter, LaserSurface |
| `ColorChangerInit` | GridPosition, (GridMover if movable), RenderTransform, ColorChanger, LaserSurface, ColorTag |

---

## Adding a New Component

Adding a component requires changes in six places:

### 1. Define the struct — `entity.h`
```cpp
struct MyComponent {
    int some_field = 0;
};
```

### 2. Add to ComponentArrays — `entity.h`
Inside `struct ComponentArrays { ... }`:
```cpp
SparseSet<MyComponent> my_component_arr;
```

Inside `void Init(int initial_capacity)`:
```cpp
my_component_arr.Init(initial_capacity);
```

Inside `void Clear()`:
```cpp
my_component_arr.Clear();
```

### 3. Insert in the relevant archetype init — `entity.h`
Find the init function for the entity type(s) that should have this component and add:
```cpp
ca->my_component_arr.Insert(id, MyComponent{ ... });
```

### 4. Save and load — `level_loader.h`
In `WriteEntityRecord`, inside the relevant entity type's branch, write the new field(s):
```cpp
WriteU32((uint32_t)mc->some_field, f);
```

In `ReadEntityRecord`, inside the corresponding `case EntityTypeV2::...` block, read the field before calling the init function:
```cpp
int some_field = (int)ReadU32(f);
// then pass it to your init function
```

### 5. Delete in editor — `game_editor.h`
In `EditorDeleteEntity`, add:
```cpp
comp_arrays.my_component_arr.Remove(id);
```
(Failure to do this causes a crash or stale data when placing entities on top of deleted ones.)

### 6. Add migration pass — `migrate_levels.cpp`
For existing level files to remain loadable, add a migration pass that inserts the new field's default bytes in-place for each affected entity type.

**Pattern**: follow the `--add-upwards-dir` or `--add-colortag` modes as a template:
1. Add a new `V2<Name>ExtraU32Count[]` array with `+1` (or however many `u32`s you're adding) for the affected entity types, 0 for all others.
2. Write a `MigrateLevelAdd<FieldName>(in_path, out_path)` function that copies each record verbatim except for affected types — those get the extra `u32`(s) inserted at the correct position.
3. Add a new `--add-<fieldname>` branch in `main()`.
4. Run the tool on all levels before shipping.

**Current field counts per entity type (in the binary record, after type+x+y):**

| Entity Type | Extra u32 fields |
|---|---|
| Player | 6: color(×4), orientation, upwards_direction |
| PushBlock | 4: color(×4) |
| StaticBlock | 0 |
| Emitter | 5: color(×4), direction |
| Receiver | 14: color(×4), channels(×10) |
| Door | 11: open_by_default, channels(×10) |
| Endgoal | 0 |
| Button | 10: channels(×10) |
| Teleporter | 5: partner_id, color(×4) |
| ColorChanger | 6: color(×4), mode, movable |

---

## Animated Sprites

### SpriteSheet

Defined in `Engine/sprite_anim.h`. Created with:
```cpp
SpriteSheet MakeSpriteSheet(Sprite sprite, int cols, int rows, int num_frames);
```

The engine globals for animated entities are in `game.cpp`:
```cpp
SpriteSheet door_open_sheet;    // 9 frames, 1 row
SpriteSheet door_close_sheet;   // 9 frames, 1 row
SpriteSheet button_down_sheet;  // 3 frames, 1 row
SpriteSheet button_up_sheet;    // 3 frames, 1 row
```

### Playing a Frame

In `entity_render.h`, `EntityRender` uses a local lambda:
```cpp
auto DrawAnimFrame = [&](SpriteSheet& sheet, float timer, float duration, ...) { ... };
```

It computes the frame index from `timer / duration * num_frames` (clamped), then sets UV uniforms to isolate that column of the sprite sheet before calling `DrawSprite`.

### Transition Animations (Door / Button)

The door uses a state machine: `DOOR_ANIM_CLOSED → DOOR_ANIM_OPENING → DOOR_ANIM_OPEN → DOOR_ANIM_CLOSING → DOOR_ANIM_CLOSED`. The `anim_timer` and `anim_duration` fields on the component drive playback.

`EntityUpdateDoor` and `EntityUpdateButton` (in `entity.h`) drive state transitions and advance the timer each frame.

In `EntityRender` the correct sprite sheet is chosen based on `anim_state`:
- `DOOR_ANIM_OPENING` → `door_open_sheet`, frame from `anim_timer`
- `DOOR_ANIM_CLOSING` → `door_close_sheet`, frame from `anim_timer`
- `DOOR_ANIM_OPEN`    → `door_open_sheet`, last frame
- `DOOR_ANIM_CLOSED`  → `door_close_sheet`, last frame (or the static closed sprite)

### Adding an Idle Animation

1. Load a sprite sheet in `Awake` inside `game.cpp`:
   ```cpp
   SpriteSheet my_sheet = MakeSpriteSheet(LoadSprite("assets/my_anim.png", shaders, gpu_buffers), cols, rows, num_frames);
   ```
2. Add a timer field to the component struct (or use a shared global timer).
3. In `EntityRender` inside `entity_render.h`, replace the static `DrawSprite(sprites[SPR_MY_ENTITY], ...)` with a `DrawAnimFrame` call, passing `fmod(global_timer, cycle_duration)` as the timer.

For a looping idle, use a global elapsed time (e.g. accumulate `dt` in `GameUpdate`) and `fmod` it against the total cycle duration.

### Adding a New Sprite

1. Drop the PNG in `Color_game_v2/build_win/assets/`.
2. Load it in `Awake`:
   ```cpp
   sprites[N] = LoadSprite("assets/my_sprite.png", shaders, gpu_buffers);
   ```
3. Add a constant in `entity_render.h`:
   ```cpp
   #define SPR_MY_SPRITE N
   ```
4. Use `DrawSprite(sprites[SPR_MY_SPRITE], rt->transform, main_camera)` inside `EntityRender`.

---

## Level Format

Levels are binary files in `build_win/levels/<name>.level`. All integers are `uint32_t` (little-endian) unless noted.

```
[width u32][height u32][num_floor_tile_types u32][num_entities u32]
[tilemap: width*height uint16 values, row-major, y=0 first]
[entity records...]
```

Each entity record:
```
[entity_type u32][pos_x u32][pos_y u32]
[type-specific fields — see table above]
```

**Entity type IDs** (`EntityTypeV2` enum in `level_loader.h`):

| ID | Type |
|---|---|
| 0 | Player |
| 1 | PushBlock |
| 2 | StaticBlock |
| 3 | Emitter |
| 4 | Receiver |
| 5 | Door |
| 6 | Endgoal |
| 7 | Button |
| 8 | Teleporter |
| 9 | ColorChanger |

Entity records are written in the order they were created (entity ID = record index). The entity ID stored in `partner_entity_id` (Teleporter) and channel arrays (Door, Button, Receiver) refers to this index.

---

## Color / Hidden Color System

`game_color_switcher.h`

### The Color Cycle

Seven colors cycle in order:

```cpp
Color hidden_color_array[] = {
    DEFAULT_BLACK, DEFAULT_RED, DEFAULT_YELLOW, DEFAULT_GREEN,
    DEFAULT_CYAN, DEFAULT_BLUE, DEFAULT_MAGENTA
};
```

`curr_hidden_color` (uint8_t) is the index of the currently hidden color. Entities with a `ColorTag` matching this color are invisible (absent from `entity_map`).

### Switching Colors (pressing C)

In `game_scenes.h`, the C key block:
1. Calls `CheckHiddenColorSwitch` — verifies no entity occupies a cell where a hidden entity would materialize. Adds shake effects to blockers. Returns false if blocked.
2. Sets iris transition globals:
   - `iris_overlay_color` = the new hidden color (painted inside the growing circle as background)
   - `iris_masked_tag_color` = the color being hidden (entities of this color are masked out inside the circle)
   - `iris_revealing_color` = the currently hidden color (entities of this color are ghost-rendered inside the circle)
3. Sets `iris_timer = IRIS_DURATION`.
4. At `iris_timer <= 0` in the timer update block, `CommitHiddenColorSwitch` is called, which advances `curr_hidden_color` and updates `entity_map`.

### isHidden

```cpp
bool isHidden(int entity_id, ComponentArrays* ca)
```

Returns true if the entity's `ColorTag` matches the current hidden color. Called at the top of `EntityRender` — hidden entities are skipped unless `force_render = true` (used by the iris ghost pass).

---

## Rendering Pipeline

`game_render.h` — `GameRender()` executes these passes each frame in order:

| Pass | What is drawn | Z range |
|---|---|---|
| 1 | Tilemap tiles | `z = -2 - 2*y` |
| 2 | Ground layer bottom halves | `z = 0 - 2*y` |
| 3 | Ground layer top halves (open doors only) | `z = 0.25 - 2*y` for doors |
| 4 | Entity layer bottom halves (skips closed doors) | `z = 1 - 2*y` |
| 5 | Emission map (laser beams) | `z = 0.5 - 2*y` |
| 6 | Bottom wall overlay (decorative front wall tiles) | `z = 2.0` |
| 7 | Entity layer top halves (all, including closed doors) | same as pass 4 |
| 8 | Iris overlay + ghost pass | drawn on top of everything |

### Z Ordering

`SetEntityZ(id, base_z, y)` sets `z = base_z - 2*y`. Higher y = smaller z = drawn further back (painter's algorithm). The `z_bump` in `EntityRender` (`+1.3f`) is applied to entities that are mid-animation across y rows, pushing them in front of entities on the row they're leaving.

### Split-Sprite Rendering

Each entity is drawn in two passes (bottom half, top half) so that entities at different y values interleave correctly at their midpoints. The UV helpers `UVBottomHalf` and `UVTopHalf` clip the sprite sheet to the lower or upper 50% of the image. The top half pass shifts the y position up by 0.5 world units so it sits in the correct tile slot.

### Iris Mask

`shader.fs` has four iris mask uniforms: `iris_mask_enabled`, `iris_mask_radius`, `iris_mask_aspect`, `iris_mask_invert`. When enabled, fragments outside the circle (or inside, if inverted) are discarded. `SetEntityIrisMask` in `game_render.h` sets these for entities being hidden during the transition.

---

## Movement & Undo

### EntityMove (`entity.h:347`)

Recursive function. Checks tile bounds, wall collision (`TestTileCollide`), teleporter redirect, closed door block, then tries to push any entity in the target cell (with `move_weight - 1`, so players can push one block but not two). On success, updates `entity_map`, `gp->prev_position`, `gp->position`, and starts the mover animation (`gm->moving = true`).

### EntityUpdateMover (`entity.h:415`)

Called every frame for each entity with `GridMover`. Lerps `RenderTransform.position` from `prev_position` to `position` over `MOVE_SPEED * 1000ms`. Clears `moving` flag on completion.

### Two-Phase Player Move Resolution (`game_scenes.h`)

The `try_move_all` lambda in `GameUpdate` implements:
1. **Per-player direction**: each player's `upwards_direction` rotates the input direction independently.
2. **Intent collection**: compute each player's target cell.
3. **Conflict detection**: same-cell conflicts and head-on swaps both block both players.
4. **Dependency graph**: if player A's target is player B's current cell, A depends on B.
5. **Cycle detection**: dependency cycles (A→B→A) block all involved.
6. **Topological sort**: commit moves in dependency order (leaves first) so a player only moves into a cell after its current occupant has moved out.

### Undo System (`game.cpp:124`)

`UndoSaveStep` saves the grid position of every `GridMover` entity before each move attempt. `UndoRevertStep` restores the last snapshot and updates `entity_map`. Buffer size is `UNDO_LENGTH = 2048` steps. Z is not part of undo — visual positions re-sync on the next `EntityUpdateMover` frame.

---

## Signal / Wiring System

Activators (receivers, buttons) and doors share integer **channel IDs**. An activator and a door are connected when they share at least one channel ID in their `SignalChannel.channels[10]` arrays. Value `-1` = unused slot.

`EntityUpdateDoor` iterates all entities, finds those with `SignalChannel`, checks if they are active receivers or pressed buttons, and checks for channel overlap. If any active activator shares a channel, the door opens.

Wires are drawn in `DrawWires` (called from render passes 3 and 7 when `showing_wires` is true), as thin rotated sprites connecting each door to its connected activators.

---

## Editor

`game_editor.h` — toggle with **Tab**.

| Action | Key |
|---|---|
| Place entity | Left click |
| Delete entity | Right click |
| Rotate facing direction | R |
| Cycle color | T + 0–7 |
| Wire activator to door | E + drag |
| Cycle tile under cursor | Q |
| Save current level | ImGui panel |
| Load a level by name | ImGui panel |

`EditorDeleteEntity` must call `.Remove(id)` on every component array. Missing a removal causes a stale component to remain and can corrupt future entity placement.

---

## Scene Management

Three scenes defined in `game.cpp`:

| ID | Constant | Update function |
|---|---|---|
| 0 | `TITLE_SCENE` | `TitleUpdate` |
| 1 | `ACT2_SCENE` | `Act2Update` |
| 2 | `GAME_SCENE` | `GameUpdate` |

Scene transitions happen inside the update functions via `scene_manager.SwitchScene(id, gs, ks, dt)`. The game starts in `TITLE_SCENE` (`Start()` in `game.cpp`).

`GameUpdate` handles: input (WASD move, R restart, Z undo, Q wire view, C color switch), per-frame entity updates (movers, buttons, receivers, doors, color changers, lasers), level win detection (player on endgoal), and level transition timing.
