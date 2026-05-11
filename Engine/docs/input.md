# Input

## Table Of Contents
- [Keyboard Input](#keyboard-input)
    - [KeyboardState struct](#keyboardstate-struct)
    - [KeyState struct](#keystate-struct)
- [Mouse Input](#mouse-input)
    - [GetMousePosition()](#getmouseposition-function)
    - [GetMousePositionInWorldCoords()](#getmousepositioninworldcoords-function)


## Keyboard Input

Getting key presses in the SkyEngine is easy. As discussed in the [Script Layout Section of the Getting Started Guide](getting_started.md#3-basic-script-layout), the `Update` and `Start` functions provide the user with a `KeyboardState` instance.

### KeyboardState struct
Contains the  `KeyState` members for the current frame and the previous frame

| Data Type             | Name       | Description |
| ----------------------- | ---------- | ----------- |
| [`KeyState`](#keystate-struct) | state      | an instance of the KeyState class that corresponds to the current frame's key states  |
| [`KeyState`](#keystate-struct) | prev_state | an instance of the KeyState class that corresponds to the previous frame's key states |

### KeyState struct


Contains a bitfield with an entry for every keyboard and mouse key. 


| Data Type          | Name         | Description |
| ------------------ | ------------ | ----------- |
| 1-bit bitfield     | MBL          | a bit that takes value 1 if the left mouse button is pressed down, and 0 otherwise |
| 1-bit bitfield     | MBR          | a bit that takes value 1 if the right mouse button is pressed down, and 0 otherwise |
| 1-bit bitfield     | MBM          | a bit that takes value 1 if the middle mouse button is pressed down, and 0 otherwise |
| 1-bit bitfield     | Q            | a bit that takes value 1 if the Q button is pressed down, and 0 otherwise |
| 1-bit bitfield     | W            | a bit that takes value 1 if the W button is pressed down, and 0 otherwise |
| 1-bit bitfield     | E            | a bit that takes value 1 if the E button is pressed down, and 0 otherwise |
| 1-bit bitfield     | R            | a bit that takes value 1 if the R button is pressed down, and 0 otherwise |
| 1-bit bitfield     | T            | a bit that takes value 1 if the T button is pressed down, and 0 otherwise |
| 1-bit bitfield     | Y            | a bit that takes value 1 if the Y button is pressed down, and 0 otherwise |
| 1-bit bitfield     | U            | a bit that takes value 1 if the U button is pressed down, and 0 otherwise |
| 1-bit bitfield     | I            | a bit that takes value 1 if the I button is pressed down, and 0 otherwise |
| 1-bit bitfield     | O            | a bit that takes value 1 if the O button is pressed down, and 0 otherwise |
| 1-bit bitfield     | P            | a bit that takes value 1 if the P button is pressed down, and 0 otherwise |
| 1-bit bitfield     | A            | a bit that takes value 1 if the A button is pressed down, and 0 otherwise |
| 1-bit bitfield     | S            | a bit that takes value 1 if the S button is pressed down, and 0 otherwise |
| 1-bit bitfield     | D            | a bit that takes value 1 if the D button is pressed down, and 0 otherwise |
| 1-bit bitfield     | F            | a bit that takes value 1 if the F button is pressed down, and 0 otherwise |
| 1-bit bitfield     | G            | a bit that takes value 1 if the G button is pressed down, and 0 otherwise |
| 1-bit bitfield     | H            | a bit that takes value 1 if the H button is pressed down, and 0 otherwise |
| 1-bit bitfield     | J            | a bit that takes value 1 if the J button is pressed down, and 0 otherwise |
| 1-bit bitfield     | K            | a bit that takes value 1 if the K button is pressed down, and 0 otherwise |
| 1-bit bitfield     | L            | a bit that takes value 1 if the L button is pressed down, and 0 otherwise |
| 1-bit bitfield     | Z            | a bit that takes value 1 if the Z button is pressed down, and 0 otherwise |
| 1-bit bitfield     | X            | a bit that takes value 1 if the X button is pressed down, and 0 otherwise |
| 1-bit bitfield     | C            | a bit that takes value 1 if the C button is pressed down, and 0 otherwise |
| 1-bit bitfield     | V            | a bit that takes value 1 if the V button is pressed down, and 0 otherwise |
| 1-bit bitfield     | B            | a bit that takes value 1 if the B button is pressed down, and 0 otherwise |
| 1-bit bitfield     | N            | a bit that takes value 1 if the N button is pressed down, and 0 otherwise |
| 1-bit bitfield     | M            | a bit that takes value 1 if the M button is pressed down, and 0 otherwise |
| 1-bit bitfield     | BRACE_LEFT   | a bit that takes value 1 if the left brace button is pressed down, and 0 otherwise |
| 1-bit bitfield     | BRACE_RIGHT  | a bit that takes value 1 if the right brace button is pressed down, and 0 otherwise |
| 1-bit bitfield     | BACKSLASH    | a bit that takes value 1 if the backslash button is pressed down, and 0 otherwise |
| 1-bit bitfield     | COLON        | a bit that takes value 1 if the colon button is pressed down, and 0 otherwise |
| 1-bit bitfield     | QUOTE        | a bit that takes value 1 if the quote button is pressed down, and 0 otherwise |
| 1-bit bitfield     | COMMA        | a bit that takes value 1 if the comma button is pressed down, and 0 otherwise |
| 1-bit bitfield     | PERIOD       | a bit that takes value 1 if the period button is pressed down, and 0 otherwise |
| 1-bit bitfield     | FORWARDSLASH | a bit that takes value 1 if the forward slash button is pressed down, and 0 otherwise |
| 1-bit bitfield     | NUM1         | a bit that takes value 1 if the num1 button is pressed down, and 0 otherwise |
| 1-bit bitfield     | NUM2         | a bit that takes value 1 if the num2 button is pressed down, and 0 otherwise |
| 1-bit bitfield     | NUM3         | a bit that takes value 1 if the num3 button is pressed down, and 0 otherwise |
| 1-bit bitfield     | NUM4         | a bit that takes value 1 if the num4 button is pressed down, and 0 otherwise |
| 1-bit bitfield     | NUM5         | a bit that takes value 1 if the num5 button is pressed down, and 0 otherwise |
| 1-bit bitfield     | NUM6         | a bit that takes value 1 if the num6 button is pressed down, and 0 otherwise |
| 1-bit bitfield     | NUM7         | a bit that takes value 1 if the num7 button is pressed down, and 0 otherwise |
| 1-bit bitfield     | NUM8         | a bit that takes value 1 if the num8 button is pressed down, and 0 otherwise |
| 1-bit bitfield     | NUM9         | a bit that takes value 1 if the num9 button is pressed down, and 0 otherwise |
| 1-bit bitfield     | NUM0         | a bit that takes value 1 if the num0 button is pressed down, and 0 otherwise |
| 1-bit bitfield     | TILDA        | a bit that takes value 1 if the tilde button is pressed down, and 0 otherwise |
| 1-bit bitfield     | MINUS        | a bit that takes value 1 if the minus button is pressed down, and 0 otherwise |
| 1-bit bitfield     | PLUS         | a bit that takes value 1 if the plus button is pressed down, and 0 otherwise |
| 1-bit bitfield     | BACKSPACE    | a bit that takes value 1 if the backspace button is pressed down, and 0 otherwise |
| 1-bit bitfield     | TAB          | a bit that takes value 1 if the tab button is pressed down, and 0 otherwise |
| 1-bit bitfield     | CAPS         | a bit that takes value 1 if the caps lock button is pressed down, and 0 otherwise |
| 1-bit bitfield     | LEFTSHIFT    | a bit that takes value 1 if the left shift button is pressed down, and 0 otherwise |
| 1-bit bitfield     | LEFTCTRL     | a bit that takes value 1 if the left control button is pressed down, and 0 otherwise |
| 1-bit bitfield     | MODKEY       | a bit that takes value 1 if the mod key button is pressed down, and 0 otherwise |
| 1-bit bitfield     | LEFTALT      | a bit that takes value 1 if the left alt button is pressed down, and 0 otherwise |
| 1-bit bitfield     | RIGHTALT     | a bit that takes value 1 if the right alt button is pressed down, and 0 otherwise |
| 1-bit bitfield     | FUNCTION     | a bit that takes value 1 if the function button is pressed down, and 0 otherwise |
| 1-bit bitfield     | RIGHTCTRL    | a bit that takes value 1 if the right control button is pressed down, and 0 otherwise |
| 1-bit bitfield     | RIGHTSHIFT   | a bit that takes value 1 if the right shift button is pressed down, and 0 otherwise |
| 1-bit bitfield     | ENTER        | a bit that takes value 1 if the enter button is pressed down, and 0 otherwise |
| 1-bit bitfield     | ESC          | a bit that takes value 1 if the escape button is pressed down, and 0 otherwise |
| 1-bit bitfield     | SPACE        | a bit that takes value 1 if the space button is pressed down, and 0 otherwise |
| 1-bit bitfield     | F1           | a bit that takes value 1 if the F1 button is pressed down, and 0 otherwise |
| 1-bit bitfield     | F2           | a bit that takes value 1 if the F2 button is pressed down, and 0 otherwise |
| 1-bit bitfield     | F3           | a bit that takes value 1 if the F3 button is pressed down, and 0 otherwise |
| 1-bit bitfield     | F4           | a bit that takes value 1 if the F4 button is pressed down, and 0 otherwise |
| 1-bit bitfield     | F5           | a bit that takes value 1 if the F5 button is pressed down, and 0 otherwise |
| 1-bit bitfield     | F6           | a bit that takes value 1 if the F6 button is pressed down, and 0 otherwise |
| 1-bit bitfield     | F7           | a bit that takes value 1 if the F7 button is pressed down, and 0 otherwise |
| 1-bit bitfield     | F8           | a bit that takes value 1 if the F8 button is pressed down, and 0 otherwise |
| 1-bit bitfield     | F9           | a bit that takes value 1 if the F9 button is pressed down, and 0 otherwise |
| 1-bit bitfield     | F10          | a bit that takes value 1 if the F10 button is pressed down, and 0 otherwise |
| 1-bit bitfield     | F11          | a bit that takes value 1 if the F11 button is pressed down, and 0 otherwise |
| 1-bit bitfield     | F12          | a bit that takes value 1 if the F12 button is pressed down, and 0 otherwise |
| 1-bit bitfield     | ARROWUP      | a bit that takes value 1 if the up arrow button is pressed down, and 0 otherwise |
| 1-bit bitfield     | ARROWDOWN    | a bit that takes value 1 if the down arrow button is pressed down, and 0 otherwise |
| 1-bit bitfield     | ARROWLEFT    | a bit that takes value 1 if the left arrow button is pressed down, and 0 otherwise |
| 1-bit bitfield     | ARROWRIGHT   | a bit that takes value 1 if the right arrow button is pressed down, and 0 otherwise |
| 1-bit bitfield     | PRINTSCREEN  | a bit that takes value 1 if the print screen button is pressed down, and 0 otherwise |
| 1-bit bitfield     | SCREENLOCK   | a bit that takes value 1 if the screen lock button is pressed down, and 0 otherwise |
| 1-bit bitfield     | PAUSE        | a bit that takes value 1 if the pause button is pressed down, and 0 otherwise |
| 1-bit bitfield     | INSERT       | a bit that takes value 1 if the insert button is pressed down, and 0 otherwise |
| 1-bit bitfield     | HOME         | a bit that takes value 1 if the home button is pressed down, and 0 otherwise |
| 1-bit bitfield     | PAGEUP       | a bit that takes value 1 if the page up button is pressed down, and 0 otherwise |
| 1-bit bitfield     | DELETE_BUTTON| a bit that takes value 1 if the delete button is pressed down, and 0 otherwise |
| 1-bit bitfield     | END          | a bit that takes value 1 if the end button is pressed down, and 0 otherwise |
| 1-bit bitfield     | PAGEDOWN     | a bit that takes value 1 if the page down button is pressed down, and 0 otherwise |

## Mouse Input

Unlike mouse button input, mouse position is not part of the `KeyboardState` struct. Instead, SkyEngine provides functions that are globally available that return the current position of the mouse cursor. This position is calculated during the function call, rather than at the start of the frame. 

There are two functios that can be used: `GetMousePosition()` and `GetMousePositionInWorldCoords`.

### GetMousePosition() function
**Signature:** `Vector2Int GetMousePosition();`

**Description:** Used to retrieve pixel-perfect mouse position.

**Parameters:** None

<!--TODO: Include a link to a description of the Vector2Int struct-->
**Return value:** [`Vector2Int`]() struct instance. The `x` and `y` correspond to pixel position. For a 1080p screen, the `{0, 0}` coordinate is the **top left** of the screen and the `{1919, 1079}` is the **bottom right** of the screen.

### GetMousePositionInWorldCoords() function
**Signature:** `Vector2 GetMousePositionInWorldCoords();`

**Description:** Used to retrieve the mouse position that corresponds to world coordinates. This is essentially like raycasting from the camera position through the cursor into the world in a 3D engine like Unity. However, since SkyEngine is 2D, this is much easier and faster to do via simple arithmetic operations, therefore no raycasting is necessary.

**Parameters:** None

<!--TODO: Include a link to a description of the Vector2 struct-->
**Return Value:** [`Vector2`]() struct instance. The `x` and `y` correspond to the world coodinates that the cursor is hovering over. If an object is centered on the `{0.0f, 0.0f}` world coordinates, and the mouse hovers over the object's origin, this will return `{0.0f, 0.0f}` regardless of screen resolution, camera position relative to the object or where on the screen the object is rendered. 


---
[Back to homepage](index.md)
