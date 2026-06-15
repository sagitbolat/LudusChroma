#pragma once

// ============================================================
// Powers granted by picking up an item.
// Gate gameplay mechanics behind these in game_scenes.h.
// ============================================================

enum class PowerGrant : int {
    None          = 0,
    ColorSwitcher = 1,
    UnlockRed     = 2,
    UnlockGreen   = 3,
    UnlockBlue    = 4,
};


// ============================================================
// Dialogue entry — one per tape/letter item.
// dialogue_id stored in the Pickup entity maps to the index here.
// ============================================================

struct DialogueEntry {
    const char* audio_paths[8]; // one clip per text line; nullptr = no audio for that line
    const char* text_lines[8];
    int         num_lines;
    PowerGrant  power;
};

// Add entries here. Index 0 = dialogue_id 0 in the editor, etc.
// audio_paths and text_lines are parallel arrays — index N plays while line N is shown.
static DialogueEntry dialogues[] = {
    // [0] example — replace with your own content
    { // 0
        { nullptr, nullptr, nullptr, nullptr, nullptr }, 
        { "Ben is a grey cube. Inside a grey world.",
          "Ben is tired of living in The Grey, so he sets out to find something others around don't believe in.",
          "'How can you find something that isn't there?!' they tell him, incregulously.",
          "So Ben leaves them behind and enters the world beyond.",
          "For Ben is looking for Color."
        },
        5,
        PowerGrant::None,
    },
    { // 1
        { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr }, 
        { "Perhaps they were right. Perhaps Ben's journey was futile.",
          "He spent many days thinking back to when he was back home.",
          "A part of him longed to go back to the safety of the past.",
          "But the other part, the part that got him to go on this journey to begin with...",
          "That part always won out in the end. And so Ben kept going.",
          "Until the hopelessness won over, and Ben was as close to giving up his search as he ever was.",
          "Until Ben was lost, and sad, and his mind was filled with self-pity and self-doubt and all he wanted was to be sad and be Blue",
          "And that was his first Color."
        },
        8,
        PowerGrant::UnlockBlue,
    },
    { // 2
        {},
        { ""

        },
        5, PowerGrant::UnlockRed
    }
    
};
const int NUM_DIALOGUES = (int)(sizeof(dialogues) / sizeof(dialogues[0]));


// ============================================================
// Persistent per-run state: which powers the player has unlocked.
// Set color_switcher_unlocked = true by default to keep all
// existing levels working without placing a pickup.
// ============================================================

struct PlayerProgress {
    bool color_switcher_unlocked = true;
    bool red_unlocked            = true;
    bool green_unlocked          = true;
    bool blue_unlocked           = true;
};

PlayerProgress player_progress = {};


// ============================================================
// Active cutscene state (one at a time).
// Driven by GameUpdate / RenderCutscene in game_scenes.h.
// ============================================================

struct SoundClip;  // forward-declared; defined in OpenAL_sound.cpp

struct CutsceneState {
    bool       active        = false;
    int        dialogue_id   = -1;
    int        current_line  = 0;
    SoundClip* clip          = nullptr;
};
