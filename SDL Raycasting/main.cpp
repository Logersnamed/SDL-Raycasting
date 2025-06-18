#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <iostream>
#include <vector>
#include <fstream>
#include <string>

constexpr double PI = 3.14159265358979323846;
constexpr int WINDOW_WIDTH = 640;
constexpr int WINDOW_HEIGHT = 480;

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;

static int windowWidth = 640;
static int windowHeight = 480;

struct vec2 {
    float x, y;

    inline vec2 operator+(const vec2& other) const { return { x + other.x, y + other.y }; }
    inline vec2 operator-(const vec2& other) const { return { x - other.x, y - other.y }; }
    inline double operator*(const vec2& other) const { return x * other.y - y * other.x; }
};

struct line {
    vec2 p1, p2;
};

struct Map {
    std::vector<line> bounds;
    int height = 100;
};

struct Player {
    vec2 pos = { 50, 50 };
    float viewAngle = 0.0f;
    int viewDistance = 150;
    int fov = 90;
    int rayCount = 360;

    float moveSpeed = (float)rayCount / 900.0f;
    float tiltSpeed = (float)rayCount / 180.0f;
};

Map map;
Player player;

void LoadMapFromFile(const char* filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        SDL_Log("Failed to open map file: %s", filename);
        return;
    }

    std::vector<std::string> lines;
    std::string line;

    while (std::getline(file, line)) {
        lines.push_back(line);
    }

    file.close();

    const float tileSize = 20.0f;

    map.bounds.clear();

    for (int y = 0; y < lines.size(); ++y) {
        for (int x = 0; x < lines[y].length(); ++x) {
            char cell = lines[y][x];

            if (cell == '#') {
                float left = x * tileSize;
                float top = y * tileSize;
                float right = left + tileSize;
                float bottom = top + tileSize;

                map.bounds.push_back({ {left, top}, {right, top} });
                map.bounds.push_back({ {right, top}, {right, bottom} });
                map.bounds.push_back({ {right, bottom}, {left, bottom} });
                map.bounds.push_back({ {left, bottom}, {left, top} });
            }
            else if (cell == 'p') {
                player.pos = { x * tileSize + tileSize / 2, y * tileSize + tileSize / 2 };
            }
        }
    }
}

void InitMap() {
    LoadMapFromFile("map.txt");
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    SDL_SetAppMetadata("SDL Raycasting", "1.0", "com.logersnamed.SDL-Raycasting");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("SDL Raycasting", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    InitMap();

    return SDL_APP_CONTINUE;
}

double toRad(double angle) {
    return angle * PI / 180.0f;
}

void renderLine(line line) {
    SDL_RenderLine(renderer, line.p1.x, line.p1.y, line.p2.x, line.p2.y);
}

void movePlayer(float dirAngle) {
	float radAngle = toRad(player.viewAngle + dirAngle);
    player.pos.x += SDL_cosf(radAngle) * player.moveSpeed;
    player.pos.y += SDL_sinf(radAngle) * player.moveSpeed;
}

double length(line line) {
    return SDL_sqrtf(SDL_powf(line.p2.x - line.p1.x, 2) + SDL_powf(line.p2.y - line.p1.y, 2));
}

bool intersects(line line1, line line2, vec2 *intersectionPoint) {
    vec2 p = line1.p1;
    vec2 q = line2.p1;
    vec2 r = line1.p2 - line1.p1;
    vec2 s = line2.p2 - line2.p1;

    float rs = r * s;

    if (rs == 0) return false;

    float qpr = (q - p) * r;
    float qps = (q - p) * s;

    float u = qpr / rs;
    float t = qps / rs;
    
    if (u >= 0 && u <= 1 && t >= 0 && t <= 1) {
        *intersectionPoint = p + vec2{ t * r.x, t * r.y };
        return true;
    }
    
    return false;
}

void raycasting() {
    float projectionConstant = (windowHeight / 2.0f) / SDL_tanf(toRad(player.fov / 2.0f));
    float angleStep = (float)player.fov / player.rayCount;
    float segmentWidth = (float)windowWidth / player.rayCount;
    for (int i = 0; i < player.rayCount; ++i) {
        float currAngle = player.viewAngle - player.fov / 2.0f + i * angleStep;

        line raycast;
        raycast.p1 = { player.pos.x, player.pos.y };
        raycast.p2 = { player.pos.x + SDL_cosf(toRad(currAngle)) * player.viewDistance, player.pos.y + SDL_sinf(toRad(currAngle)) * player.viewDistance };

        vec2 intersection;
        bool intersectionFound = false;
        for (line bound : map.bounds) {
            vec2 currIntersect;
            if (intersects(raycast, bound, &currIntersect)) {
                if (!intersectionFound) {
                    intersection = currIntersect;
                }
                else {
                    float lenghtPrev = length({ intersection, player.pos });
                    float lenghtCurr = length({ currIntersect, player.pos });
                    intersection = lenghtCurr < lenghtPrev ? currIntersect : intersection;
                }
                intersectionFound = true;
            }
        }

        if (!intersectionFound) {
            SDL_SetRenderDrawColor(renderer, 120, 120, 120, SDL_ALPHA_OPAQUE);
            renderLine(raycast);
            continue;
        }

        float dist = length({ player.pos, intersection });
        if (dist == 0) continue;

        float wallHeight = (map.height / dist) * projectionConstant;
        float upheight = wallHeight / 2.0f;

        SDL_FRect segment;
        segment.x = segmentWidth * i;
        segment.w = segmentWidth;
        segment.y = (windowHeight / 2.0f) - upheight;
        segment.h = wallHeight;

        int lightness = 255 - dist * 2;
        lightness = lightness > 255 ? 255 : lightness;
        lightness = lightness < 0 ? 0 : lightness;
        SDL_SetRenderDrawColor(renderer, lightness, lightness, lightness, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(renderer, &segment);

        SDL_SetRenderDrawColor(renderer, 120, 120, 120, SDL_ALPHA_OPAQUE);
        raycast.p2 = intersection;
        renderLine(raycast);
    }
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    if (event->type == SDL_EVENT_WINDOW_RESIZED) {
        windowWidth = event->window.data1;
        windowHeight = event->window.data2;
    }
    return SDL_APP_CONTINUE;
}

void handleInput() {
    const bool* keystate = SDL_GetKeyboardState(NULL);

    player.moveSpeed = (float)player.rayCount / (keystate[SDL_SCANCODE_LSHIFT] ? 450.0f : 900.0f);

    if (keystate[SDL_SCANCODE_W]) movePlayer(0);
    if (keystate[SDL_SCANCODE_S]) movePlayer(180);
    if (keystate[SDL_SCANCODE_A]) movePlayer(270);
    if (keystate[SDL_SCANCODE_D]) movePlayer(90);

    if (keystate[SDL_SCANCODE_Q]) player.viewAngle -= player.tiltSpeed;
    if (keystate[SDL_SCANCODE_E]) player.viewAngle += player.tiltSpeed;

    if (keystate[SDL_SCANCODE_F]) keystate[SDL_SCANCODE_LSHIFT] ? --player.fov : ++player.fov;
    if (keystate[SDL_SCANCODE_H]) keystate[SDL_SCANCODE_LSHIFT] ? --map.height : ++map.height;
    if (keystate[SDL_SCANCODE_V]) keystate[SDL_SCANCODE_LSHIFT] ? --player.rayCount : ++player.rayCount;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
	handleInput();

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    raycasting();

    SDL_SetRenderDrawColor(renderer, 120, 120, 120, SDL_ALPHA_OPAQUE);
    for (line bound : map.bounds) {
        renderLine(bound);
    }

    SDL_RenderPoint(renderer, player.pos.x, player.pos.y);

    SDL_RenderPresent(renderer);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    
}