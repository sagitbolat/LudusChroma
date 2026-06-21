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
    const char* audio_paths[16]; // one clip per text line; nullptr = no audio for that line
    const char* text_lines[16];
    int         num_lines;
    PowerGrant  power;
};

// Add entries here. Index 0 = dialogue_id 0 in the editor, etc.
// audio_paths and text_lines are parallel arrays — index N plays while line N is shown.
static DialogueEntry dialogues[] = {
    // [0] example — replace with your own content
    { // 0
        { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr }, 
        { "There once was a boy, who was being lied to.",
          "Why was he being lied to?",
          "He lived in a world of Color, but all he saw was shades of Grey.",
          "Did he find the truth?",
          "He went out to search for something he could not yet understand.",
          "But once he did understand, he wished he could forget."
        },
        6,
        PowerGrant::None,
    },
    { // 1
        { nullptr, nullptr, nullptr, nullptr}, 
        { "The boy found a piece of truth. The truth made him lonely.",
          "Why lonely?",
          "He knew he could not share the truth, for nobody would understand.",
          "With nothing to show for his hard work, he left the fire and the cave behind.",
        },
        4,
        PowerGrant::UnlockBlue,
    },
    { // 2
        {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr},
        { "Faster than he thought possible, his understanding of the world grew.",
          "It was almost as if having the taste of truth started a cascade of understanding.",
          "No longer did he feel blind, but had a clear idea of what there is out there.",
          "But with knowledge, came arrogance. With arrogance came expectation.",
          "His progress slowed, and the final peice was still out of reach.",
          "And so he grew distant from the world. Hateful of those around him for not understanding.",
          "But most crucially, hateful of himself for not being good enough to explain."
        },
        7, PowerGrant::UnlockRed
    },
    { // 3
        {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr},
        { "The boy had thought knowledge would bring answers and solutions."
          "Instead it brought silence. And in the silence he found peace.",
          "That silence, an echo of every step he took to get here.",
          "The Loneliness was not a curse. The Anger not a mistake.",
          "They were colors too.",
          "And once he stopped trying to erase them, he finally understood their unity.",
          "'I am them' the boy said.",
          "And so he was them.",
          "And they were he?",
          "And they were he."
        },
        10, PowerGrant::UnlockGreen
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
    float      text_timer    = 0.f;  // ms elapsed on the current line
    SoundClip* clip          = nullptr;
};
