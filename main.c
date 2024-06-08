#include <stdio.h>
#include <SDL.h>
#include <stdbool.h>

// Simulation size params
//#define GRID_SIZE_X 1920
//#define GRID_SIZE_Y 1080
#define GRID_SIZE_X 1920
#define GRID_SIZE_Y 1080
#define NUM_BOIDS 500

// Behavior params
#define MAX_VELOCITY 6
#define MIN_VELOCITY 3
#define PROTECTED_RANGE 8
#define VISIBLE_RANGE 40
#define AVOID_FACTOR 0.05f
#define MATCH_FACTOR 0.05f
#define CENTERING_FACTOR 0.0005f

#define MARGIN 100
#define TURN_FACTOR 0.2f


// update every n frames
#define SIMULATION_SPEED 5

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;

typedef struct boid {
    int x;
    int y;
    double vx;
    double vy;

    uint8_t r;
    uint8_t g;
    uint8_t b;
} boid_t;
boid_t boids[2][NUM_BOIDS];
int active = 0;
int inactive = 1;

bool should_quit = false;

void handle_event(SDL_Event *event) {
  switch (event->type) {
  case SDL_QUIT:
    should_quit = true;
    break;
  case SDL_KEYDOWN:
    switch (event->key.keysym.sym) {
    case SDLK_ESCAPE:
      should_quit = true;
      break;
    }
    break;
  }
}

int clamp(int val, int max) {
    if (val < 0) {
        return 0;
    } else if (val > max) {
        return max;
    } else {
        return val;
    }
}

boid_t apply_velocity(boid_t boid) {
    boid_t new = boid;
    new.x = boid.x + new.vx;
    new.y = boid.y + new.vy;
    return new;
}

double distance(boid_t* boid1, boid_t* boid2) {
    return sqrt(pow(boid2->x - boid1->x, 2) + pow(boid2->y - boid1->y, 2));
}

void update() {
    for (int i = 0; i < NUM_BOIDS; i++) {
        boid_t* boid = &boids[active][i];
        double original_vx = boid->vx;
        double original_vy = boid->vy;

        double close_dx = 0;
        double close_dy = 0;

        int num_neighbors = 0;
        int neighbor_x_total = 0;
        int neighbor_y_total = 0;
        double neighbor_vx_total = 0;
        double neighbor_vy_total = 0;

        for (int j = 0; j < NUM_BOIDS; j++) {
            boid_t* other = &boids[active][j];
            double dist = distance(boid, other);
            if (dist < PROTECTED_RANGE) {
                close_dx += boid->x - other->x;
                close_dy += boid->y - other->y;
            }

            if (dist < VISIBLE_RANGE) {
                num_neighbors++;
                neighbor_x_total += other->x;
                neighbor_y_total += other->y;
                neighbor_vx_total += other->vx;
                neighbor_vy_total += other->vy;
            }
        }

        boid->vx += close_dx * AVOID_FACTOR;
        boid->vy += close_dy * AVOID_FACTOR;

        if (num_neighbors > 0) {
            double neighbor_x_avg = neighbor_x_total / (double)num_neighbors;
            double neighbor_y_avg = neighbor_y_total / (double)num_neighbors;
            double neighbor_vx_avg = neighbor_vx_total / (double)num_neighbors;
            double neighbor_vy_avg = neighbor_vy_total / (double)num_neighbors;

            boid->vx += (neighbor_x_avg - boid->x) * CENTERING_FACTOR;
            boid->vy += (neighbor_y_avg - boid->y) * CENTERING_FACTOR;

            boid->vx += (neighbor_vx_avg - boid->vx) * MATCH_FACTOR;
            boid->vy += (neighbor_vy_avg - boid->vy) * MATCH_FACTOR;
        }

        if (boid->x < 0 + MARGIN) {
            boid->vx += TURN_FACTOR;
        } else if (boid->x > GRID_SIZE_X - MARGIN) {
            boid->vx -= TURN_FACTOR;
        }

        if (boid->y < 0 + MARGIN) {
            boid->vy += TURN_FACTOR;
        } else if (boid->y > GRID_SIZE_Y - MARGIN) {
            boid->vy -= TURN_FACTOR;
        }

        if (boid->vx == original_vx && boid->vy == original_vy) {
            double tweak = rand();
            tweak /= (double)RAND_MAX;
            tweak = (tweak - 0.5) * 0.1;
            boid->vx += (boid->vx * tweak);
            boid->vy += (boid->vy * tweak);
        }

        double speed = sqrt(pow(boid->vx, 2) + pow(boid->vy, 2));
        if (speed > MAX_VELOCITY) {
            boid->vx = ((double)boid->vx / speed) * MAX_VELOCITY;
            boid->vy = ((double)boid->vy / speed) * MAX_VELOCITY;
        } else if (speed < MIN_VELOCITY) {
            boid->vx = ((double)boid->vx / speed) * MIN_VELOCITY;
            boid->vy = ((double)boid->vy / speed) * MIN_VELOCITY;
        }

        boids[inactive][i] = apply_velocity(*boid);
    }

    int temp = active;
    active = inactive;
    inactive = temp;
}

bool on_screen(boid_t* boid) {
    if (boid->x > GRID_SIZE_X || boid->x < 0 || boid->y > GRID_SIZE_Y || boid->y < 0) {
        return false;
    } else {
        return true;
    }
}

void draw() {
    // Clear screen black
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);


    // Draw boids
    // Boids are drawn as a line from their previous position to their current position
    for (int i = 0; i < NUM_BOIDS; i++) {
        boid_t* old = &boids[inactive][i];
        boid_t* new = &boids[active][i];
        if (on_screen(old) && on_screen(new)) {
            SDL_SetRenderDrawColor(renderer, old->r, old->g, old->b, 255);
            SDL_RenderDrawLine(renderer, old->x, old->y, new->x, new->y);
        }
    }

    SDL_RenderPresent(renderer);
}

int main(int argc, char** argv) {
    // init sdl
    int res = SDL_Init(SDL_INIT_VIDEO);
    printf("SDL initialized with code %d\n", res);
    window = SDL_CreateWindow("boids", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, GRID_SIZE_X, GRID_SIZE_Y, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // init boids
    for (int i = 0; i < NUM_BOIDS; i++) {
        boids[inactive][i].x = rand() % GRID_SIZE_X;
        boids[inactive][i].y = rand() % GRID_SIZE_Y;

        boids[inactive][i].vx = rand() % MAX_VELOCITY;
        boids[inactive][i].vy = rand() % MAX_VELOCITY;

        boids[inactive][i].r = rand() % 256;
        boids[inactive][i].g = rand() % 256;
        boids[inactive][i].b = rand() % 256;

        boids[active][i] = apply_velocity(boids[inactive][i]);
    }

    int update_counter = 0;
    SDL_Event event;
    while(!should_quit) {
        while (SDL_PollEvent(&event)) {
            handle_event(&event);
        }
        if (++update_counter > SIMULATION_SPEED) {
            update();
            update_counter = 0;
        }
        draw();
    }
    
    return 0;
}
