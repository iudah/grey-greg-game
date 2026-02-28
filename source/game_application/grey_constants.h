#ifndef GREY_CONSTANTS_H
#define GREY_CONSTANTS_H

#include <stdbool.h>

// ------ Numeric precision --------------------------------------------------------------------------------------------------------------
#define GREY_ZERO             (1e-5f)
#define GREY_COLLISION_GAP    (GREY_ZERO * 1e2f)

// ------ Entity / component capacity ------------------------------------------------------------------------------------------
#define INITIAL_CAPACITY      (8)
#define MAX_NO_ENTITY         (1024)

// ------ World / tile ------------------------------------------------------------------------------------------------------------------------
#define GREY_TILE_SIZE        (16)       // pixels per tile

// ------ Spatial hash ------------------------------------------------------------------------------------------------------------------------
#define SPATIAL_CELL_SIZE     (GREY_TILE_SIZE)
#define GREY_TABLE_SIZE       (13297)    // prime → fewer hash collisions

// Max cells a single entity can span per axis.
// A 1-tile entity straddles at most 2 cells. Raise for larger entities.
#define SPATIAL_MAX_CELL_SPAN (2)

// Bucket sizing. CELL_GROW_STEP must be a power of 2 to use bitwise growth detection in add_entity_to_cell.
#define CELL_INITIAL_CAPACITY (32)
#define CELL_GROW_STEP        (32)
#define CELL_GROW_MASK        (CELL_GROW_STEP - 1)  

// Spatial hash multiply constants (chosen for bit dispersion)
#define HASH_PRIME_X          (92837111u)
#define HASH_PRIME_Y          (689287499u)
#define HASH_PRIME_Z          (3892886149u)

// ------ Physics ----------------------------------------------------------------------------------------------------------------------------------
#define GRAVITATIONAL_ACCEL   (9.8f)
#define MAX_ACCUMULATED_TIME  (0.25)     // spiral-of-death guard (seconds)

// ------ AI distances ------------------------------------------------------------------------------------------------------------------------
#define CHASE_RADIUS          (5.0f)
#define CHASE_LOST_RADIUS     (7.5f)
#define FLEE_RADIUS           (2.0f)
#define ATTACK_RANGE          (1.0f)
#define WAYPOINT_THRESHOLD    GREY_ZERO

// ------ AI speeds ------------------------------------------------------------------------------------------------------------------------------
#define FLEE_SPEED            (0.5f)
#define CHASE_SPEED           (0.45f)
#define PATROL_SPEED          (0.4f)

// ------ Player movement ------------------------------------------------------------------------------------------------------------------
#define XY_ACCEL              (40.f)
#define JUMP                  (140.f)

// ------ Event system ------------------------------------------------------------------------------------------------------------------------
#define MAX_EVENT             (63555)
#define MAX_EVENT_HANDLER     (25)

// ------ System manager --------------------------------------------------------------------------------------------------------------------
#define MAX_SYSTEM            (32)

// ------ Rendering ------------------------------------------------------------------------------------------------------------------------------
#define SPRITE_X              (8)        // half-extent in pixels
#define SPRITE_Y              (8)

// ------ Dimension mode --------------------------------------------------------------------------------------------------------------------
// Declared in game_main.c, defined here for documentation.
// Use use_2d() to set at startup. Controls collision axis count,
// ray-box tests, sweep-and-prune z clamping, and spatial hash z lane.
extern bool _is_2d;         
static inline bool grey_is_2d(void) { return _is_2d; }
void use_2d(void);

// ------ Collision layers ----------------------------------------------------------------------------------------------------------------
// Bit flags. An entity collides with another when:
//   (layer_a & mask_b) != 0 && (layer_b & mask_a) != 0
typedef enum {
  COLLISION_LAYER_NONE    = 0,
  COLLISION_LAYER_PLAYER  = 1 << 0,
  COLLISION_LAYER_TERRAIN = 1 << 1,
  COLLISION_LAYER_NPC     = 1 << 2,
  // next: 1 << 3 ... up to 1 << 31
} collision_layer;

// ------ Tile flags ----------------------------------------------------------------------------------------------------------------------------
typedef enum {
  TILE_UNKNOWN   = 0,
  TILE_SOLID     = 1 << 0,
  TILE_WALKABLE  = 1 << 1,
  TILE_HAZARD    = 1 << 2,
  TILE_CLIMBABLE = 1 << 3,
} resc_tile_flag;

// ------ AI states ------------------------------------------------------------------------------------------------------------------------------
typedef enum {
  AI_IDLE,
  AI_PATROL,
  AI_CHASE,
  AI_ATTACK,
  AI_FLEE,
} ai_state;

// ------ WFC directions --------------------------------------------------------------------------------------------------------------------
typedef enum {
  NORTH,
  EAST,
  SOUTH,
  WEST,
  NORTH_EAST,
  SOUTH_EAST,
  SOUTH_WEST,
  NORTH_WEST,
  N_DIRECTION,
} wfc_direction;

// ------ Input pad / stick indices ----------------------------------------------------------------------------------------------
typedef enum {
  PAD_A,
  PAD_X,
  PAD_B,
  PAD_O,
  PAD_COUNT,
} PadIndex;

typedef enum {
  STICK_KNOB,
  STICK_BASE,
  STICK_COUNT,
} StickIndex;

#endif // GREY_CONSTANTS_H