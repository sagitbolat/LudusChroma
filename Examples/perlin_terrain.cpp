#include "Engine/SDL_sky.cpp"


#include "Engine/bmp.cpp"
#include "Engine/simplex_noise.cpp"

#include "array_utils.cpp"

int cells_width = 1280;
int cells_height = 720;
int* cells;


// Octaves >= 1, magnification != integer value.
void GenerateCells(int octaves, float magnification, float elevation_cutoff = 0) {
    for (int y = 0; y < cells_height; ++y) {
    	for (int x = 0; x < cells_width; ++x) {
	    int val = 0;
    	if (Noise::simplex_fractal(octaves, (float)x / magnification, (float)y / magnification, 1280.0f / magnification) > elevation_cutoff) {
		    val = 255;
	    }
	    cells[y * cells_width + x] = val;
	}	
    }
}

void Init(int* w, int* h) {
    *w = 1280;
    *h = 720;
    return;
}


void Awake(GameMemory* gm) {
    cells = (int*)ArenaAllocateAsset(&gm->permanent_storage, sizeof(int) * cells_width * cells_height);
}

void Start(GameState* gs, KeyboardState* ks) {
    GenerateCells(1, 1000.0f);
    int imin = ArrMin(cells, cells_width * cells_height);
    int imax = ArrMax(cells, cells_width * cells_height);
    printf("Min: %d\n", imin);
    printf("Max: %d\n", imax);
}

void Update(GameState* gs, KeyboardState* ks, int dt) {
    static int octaves = 1;
    static float mag = 1000.0f;
    static float elev_cutoff = 0.0f;
    if (ks->state.D == 1 && ks->prev_state.D == 0) {
    	octaves += 1;
        GenerateCells(octaves, mag, elev_cutoff);
	    printf("Current octaves: %d\n", octaves);
    } else if (ks->state.A == 1 && ks->prev_state.A == 0) {
    	octaves -= 1;
        GenerateCells(octaves, mag, elev_cutoff);
	    printf("Current octaves: %d\n", octaves);
    }
    if (ks->state.W == 1 && ks->prev_state.W == 0) {
        Noise::lacunarity += 0.1f;
        GenerateCells(octaves, mag, elev_cutoff);
        printf("Current lacunarity: %f\n", Noise::lacunarity);
    } else if (ks->state.S == 1 && ks->prev_state.S == 0 && Noise::lacunarity >= 1.1f) {
        Noise::lacunarity -= 0.1f;
        GenerateCells(octaves, mag, elev_cutoff);
        printf("Current lacunarity: %f\n", Noise::lacunarity);
    }
    if (ks->state.E == 1 && ks->prev_state.E == 0) {
        elev_cutoff += 0.1f;
        GenerateCells(octaves, mag, elev_cutoff);
        printf("Current elevation is: %f\n", elev_cutoff);
    } else if (ks->state.Q == 1 && ks->prev_state.Q == 0) {
        elev_cutoff -= 0.1f;
        GenerateCells(octaves, mag, elev_cutoff);
        printf("Current elevation is: %f\n", elev_cutoff);
    }
    if (ks->state.R == 1 && ks->prev_state.R == 0) {
        mag += 10.0f;
        GenerateCells(octaves, mag, elev_cutoff);
        printf("Current magnification: %f\n", mag);
    } else if (ks->state.F == 1 && ks->prev_state.F == 0) {
        mag -= 10.0f;
        GenerateCells(octaves, mag, elev_cutoff);
        printf("Current magnification: %f\n", mag);
    }
	
    for (int y = 0; y < cells_height; ++y) {
    	for (int x = 0; x < cells_width; ++x) {
            int val = cells[y * cells_width + x];
            if (val > 1 || val < -1) {
                //printf("%f\n", val);
            } 
            DrawPixel(graphics_buffer, (uint8_t) (val), (uint8_t) (val), (uint8_t) (val), x, y);	
	
	    }
    }

}

void UserFree() {

}
