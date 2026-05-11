# Key:
* [ ] - A todo tickbox
    * [ ] - Did not start working on it
    * [?] - Possible thing to do, need to think about it more
    * [V] - Done.
    * [O] - Working on it now or need to start working on it soon. 

# TODO NOW:

* [ ] Take out UI, OpenAL, and OpenGL files into their own translation units, so they can be compiled seperately, since compile time is so long right now. Will also help with abstraction since internal OpenGL, OpenAL, and ImGui functions will not be exposed in the include header, and therefore be unavailable to the rest of the engine and the end user of the engine.

* [O] Implement common data sctuctures and put them in the "Engine/Datastructs/.." folder as includes for the user:
    * Dynamic Array
        * Make a 2D version?
    * Linked list
        * Singly-linked
        * Doubly-linked
    * Graph 
        * Directed
        * Undirected
    * Trees
        * Binary
* [O]Refactor allocation:
    * Rename ArenaAllocator to BumpArena and FreeListAllocator to FreeListArena
    * Make a dedicated alloc method that you can pass any arena too and it will allocate to it based on what type of arena it is (function overloading)
    * Make a dedicated free method that will free an arena
    * Make a dedicated memset method that will memset an arena's memory to a value (such as clearing all memory to 0)
    * Make a dedicated InitArena method that will init whatever arena is passed to it. 




# TODO (for later):


# DONE:
* Take out all struct and data types into a seperate "skytypes.h" that will be included in every file.
* Make dt more persise than just ms. Need more something like microseconds.
* 2D collisions.

### Features to implement:
* 2D kinematics.
* Physics engine. (simple kinematics only for now).
* Sound

### Refactoring
* Replace SDL with OpenGL. 
    * OpenGL is good for 2D.
    * Look into which version to use. Probably looks like 3.3 is good?
    * Pair it with SDL for window management. 

* For asset loading, implement a FIFO allocator. So that old assets can be flushed out in favour of new ones. In case asset size and level size are big, and not enough memory is
available for the asset arena. 

* Add mersenne twister and permutated conguential generator(PCG64) to the PRNG module.

### Bugs

### Assets
* Create a converter that will convert bitmaps from assets (bmps and ttfs)
into a custom asset file. 
    a. We can load the custom asset file on our shipped builds instead of having
    seperate bmp and ttf parsers that do it on runtime. Since loading the 
    custom asset file will be faster than a raw bmp or ttf, we can save on frame
    time this way.






# Color Puzzle Game Ideas
* You unlock colors as you progress. First you start in black and white. Then you unlock a color channel (continous 0.0 - 1.0). You then unlock a second channel, so you now have combinations of red-green, or blue-green, or red-blue. Finally you unlock all 3 color channels. Eventually, you unlock the alpha channel, which will let you make objects disappear.

* The gameworld is a 2D tilemap with tile objects that emit lasers of different colors (depending on the number of channels you have unlocked). Other tile objects can either reflect or redirect or fork or recolor the lasers. You need to rotate, move or construct these objects to connect the laser emitter signal to appropriate recievers. Each reciever requires a specific laser color.

* Higher level idea: The progression from 1-bit to monochrome to colored represents a graphical timeline similar to how computer graphics developed as the art of game dev grew. Similarly, the sound track should reflect that with the level sound tracks being classical music from periods between pre-1400s to renaissants to classical, played first with only wind and percussion instruments, then moving to string then eventually more recent instruments like piano.

# RPG Game ideas:

## Graphical styles:
* Instead of having bitmaps such that pixels are adjacent, leave a gap between the pixels, so that the seperation between pixels is visible. It looks like a cool way to render pixels.
* Example of a 3x3 bitmap(P represents a filled pixel, and O represents an empty space):
    P O P O P
    O O O O O
    P O P O P
    O O O O O
    P O P O P
* as opposed to:
    P P P
    P P P
    P P P



## Map Generation

### Overworld Gen:
#### Create a simulation of real world continent forming
1. Generate a Pangea
2. Separate the Pangea into smaller continents
3. Create continental drift
4. Smaller continents generate from the drift and fractures + tectonic activity.
    * **Types: (https://www.youtube.com/watch?v=3ZpDjdFzQUM&ab_channel=MooMooMathandScience)**
    * Convergent:
        * Continental + continental = mountains
        * Continental + oceanic = volcanoes.
        * Oceanic + oceanic = Island arc + marine trench
    * Divergent:
        * oceanic + oceanic = new ocean floor + volcanoes
        * continental + continental = trough.
    * Transform:
        * earthquakes
        * The movement is not constant. its sporadic. 
    * https://www.youtube.com/watch?v=R_pDKyg5YKY&ab_channel=ClickView


* https://en.wikipedia.org/wiki/Plate_tectonics

**Implementation:**
1. Form a Pangaea:
    * DLA algorithm?
    * Cellular Automaton algorithm?
2. Seperate the pangea into smaller continents:
    * run voronoi diagrams algorithm to seperate tectonic plates. 
3. Create continental drift:
    * move each region in a random direction (only the cardinal directions?)
    * determine the type of fault at each edge depending on the directions, and form the specified features. 
4. Shift the centers of the voronoi centers slightly and regenerate the diagram (step 2) and repeat step 3.

**Questions to research:**
* How do Lakes form?
* How do rivers form?
* How are biomes formed? 
