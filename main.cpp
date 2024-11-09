/**
* Author: Jemima Datus
* Assignment: Rise of the AI
* Date due: 2024-11-09, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define STB_IMAGE_IMPLEMENTATION //need for textures


#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"                // 4x4 Matrix
#include "glm/gtc/matrix_transform.hpp"  // Matrix transformation methods
#include "ShaderProgram.h"               // We'll talk about these later in the course
#include "stb_image.h"
#include "Entity.h"
#include "Map.h"
#include <vector>
#include <string>
#include <SDL_mixer.h>

enum AppStatus { RUNNING, TERMINATED };

// Our window dimensions
constexpr int WINDOW_WIDTH = 640 * 1.5f,
WINDOW_HEIGHT = 480 * 1.5f;

// Background color components
constexpr float BG_RED = 0.875f,
BG_BLUE = 0.965f,
BG_GREEN = 0.961f,
BG_OPACITY = 1.0f;

// Our viewport or our "camera"'s position and dimensions
constexpr int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

// Texture Shaders
constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";


AppStatus g_app_status = RUNNING;
SDL_Window* g_display_window;

ShaderProgram g_shader_program;

//variables for matrices
glm::mat4 g_view_matrix, g_model_matrix, g_projection_matrix;

//texture id variable
GLuint g_fontSheetTextureID;
GLuint g_test;

//variables for sprite textures
//player
constexpr char g_alienWalk[] = "alienGreen_walk.png";
constexpr char g_alienStand[] = "alienGreen_stand.png";
constexpr char g_alienJump[] = "alienGreen_jump.png";
//enemies
constexpr char g_frog[] = "frog.png";
constexpr char g_frogLeap[] = "frog_leap.png";
constexpr char g_ghost[] = "ghost.png";
constexpr char g_slime[] = "slime.png";
constexpr char g_slimeWalk[] = "slime_walk.png";
//general
constexpr char g_fontSheet[] = "font1.png";
constexpr char g_mapTiles[] = "tilemap.png";
bool g_won = false;

//variables for load_texture
constexpr int NUMBER_OF_TEXTURES = 1;
constexpr GLint LEVEL_OF_DETAIL = 0;
constexpr GLint TEXTURE_BORDER = 0;

//variable for drawing text
constexpr int fontBankSize = 16;

//map data
unsigned int mapData[] =
{
    12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12,
    32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 32,
    52, 0, 0, 0, 0, 0, 21, 22, 22, 23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52,
    72, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 72,
    21, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23,
    121, 122, 122, 122, 122, 122, 122, 122, 122, 122, 122, 122, 122, 122, 122, 122, 122, 122, 122, 123
};

int mapWidth = 20;
int mapHeight = 6;
int numEnemies = 3;

//music variables
constexpr int CD_QUAL_FREQ = 44100,
AUDIO_CHAN_AMT = 2,
AUDIO_BUFF_SIZE = 4096;
constexpr char BGM_FILEPATH[] = "Canon.mp3";
constexpr int    LOOP_FOREVER = -1;  //-1 so that it will loop forever
Mix_Music* g_music;

constexpr int PLAY_ONCE = 0, NEXT_CHNL = -1;
Mix_Chunk* g_jumpSound;

//Game state Struct
struct GameState {
    Entity* player;
    Entity* enemies;
    Map* map;
};

//game state variable 
GameState g_gameState;

//variable for delta time and fixed time step stuff
float g_prevTicks = 0.0f;
float g_fixed_time_step = 1 / 60.0f;
float g_time_accumulator = 0.0f;

//load texture function
GLuint loadTexture(const char* filePath) {

    int width, height, numberOfComponents;
    unsigned char* image = stbi_load(filePath, &width, &height, &numberOfComponents, STBI_rgb_alpha);

    //show error message if the image was not loaded
    if (image == NULL) {
        std::cerr << "Unable to load image. Make sure the path is correct.";
        assert(false);
    }

    //generating a textureID and binding it to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    //free up memory allocated for image file data
    stbi_image_free(image);

    return textureID;

}
//draw text function
void draw_text(ShaderProgram* shader_program, GLuint font_texture_id, std::string text,
    float font_size, float spacing, glm::vec3 position)
{
    // Scale the size of the fontbank in the UV-plane
    // We will use this for spacing and positioning
    float width = 1.0f / fontBankSize;
    float height = 1.0f / fontBankSize;

    // Instead of having a single pair of arrays, we'll have a series of pairs?one for
    // each character. Don't forget to include <vector>!
    std::vector<float> vertices;
    std::vector<float> texture_coordinates;

    // For every character...
    for (int i = 0; i < text.size(); i++) {
        // 1. Get their index in the spritesheet, as well as their offset (i.e. their
        //    position relative to the whole sentence)
        int spritesheet_index = (int)text[i];  // ascii value of character
        float offset = (font_size + spacing) * i;

        // 2. Using the spritesheet index, we can calculate our U- and V-coordinates
        float u_coordinate = (float)(spritesheet_index % fontBankSize) / fontBankSize;
        float v_coordinate = (float)(spritesheet_index / fontBankSize) / fontBankSize;

        // 3. Inset the current pair in both vectors
        vertices.insert(vertices.end(), {
            offset + (-0.5f * font_size), 0.5f * font_size,
            offset + (-0.5f * font_size), -0.5f * font_size,
            offset + (0.5f * font_size), 0.5f * font_size,
            offset + (0.5f * font_size), -0.5f * font_size,
            offset + (0.5f * font_size), 0.5f * font_size,
            offset + (-0.5f * font_size), -0.5f * font_size,
            });

        texture_coordinates.insert(texture_coordinates.end(), {
            u_coordinate, v_coordinate,
            u_coordinate, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate + width, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate, v_coordinate + height,
            });
    }

    // 4. And render all of them using the pairs
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);

    shader_program->set_model_matrix(model_matrix);
    glUseProgram(shader_program->get_program_id());

    glVertexAttribPointer(shader_program->get_position_attribute(), 2, GL_FLOAT, false, 0,
        vertices.data());
    glEnableVertexAttribArray(shader_program->get_position_attribute());

    glVertexAttribPointer(shader_program->get_tex_coordinate_attribute(), 2, GL_FLOAT,
        false, 0, texture_coordinates.data());
    glEnableVertexAttribArray(shader_program->get_tex_coordinate_attribute());

    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

    glDisableVertexAttribArray(shader_program->get_position_attribute());
    glDisableVertexAttribArray(shader_program->get_tex_coordinate_attribute());
}


void initialise()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow("Rise of AI",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    if (g_display_window == nullptr)
    {
        std::cerr << "ERROR: SDL Window could not be created.\n";
        g_app_status = TERMINATED;

        SDL_Quit();
        exit(1);
    }
    //start audio
    Mix_OpenAudio(CD_QUAL_FREQ, MIX_DEFAULT_FORMAT, AUDIO_CHAN_AMT, AUDIO_BUFF_SIZE);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    // Initialise our camera
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    // Load up our shaders
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    //enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initialise our view, model, and projection matrices
    g_view_matrix = glm::mat4(1.0f);  // Defines the position (location and orientation) of the camera
    g_model_matrix = glm::mat4(1.0f);  // Defines every translation, rotations, or scaling applied to an object
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);  // Defines the characteristics of your camera, such as clip planes, field of view, projection method etc.


    //load font sheet texture id
    g_fontSheetTextureID = loadTexture(g_fontSheet);

    //create our map on the heap
    g_gameState.map = new Map(mapWidth, mapHeight, mapData, loadTexture(g_mapTiles), 1.0f, 20, 9);
 
    //create our player on the heap
    std::vector<std::vector<int>> animationsPlayer = {
        {0}, //standing
        {0}, //jumping
        {0, 1} //walking
    };
    std::vector<GLuint> textureIdsPlayer = { loadTexture(g_alienStand), loadTexture(g_alienJump), loadTexture(g_alienWalk) };
    g_gameState.player = new Entity(
        textureIdsPlayer, //textureID
        1.0f, //speed
        3.0f, //jump power
        animationsPlayer, //animations
        0.0f, //animation time
        1, //frames
        0, //animation index
        1, //cols
        1, //rows
        IDLE, //animation
        PLAYER, //entity type
        1.0f, //width
        1.0f //height
        );
    g_gameState.player->set_position(glm::vec3(1.0f, -1.75f, 0.0f));
    //create enemies
    g_gameState.enemies = new Entity[numEnemies];
    //creating frog enemey
    std::vector<GLuint> textureIdsFrog = { loadTexture(g_frog), loadTexture(g_frogLeap)};
    std::vector<std::vector<int>> animationsFrog = {
        {0}, //idle
        {0}, //jumping
    };
    g_gameState.enemies[0] = Entity(textureIdsFrog, 1.0f, 1.8f, animationsFrog, 0.0f, 1, 0, 1, 1, IDLE, ENEMY, 0.81f, 0.81f);
    g_gameState.enemies[0].set_ai_type(FROG);
    g_gameState.enemies[0].set_ai_state(IDLING);
    g_gameState.enemies[0].set_animation_state(IDLE);
    g_gameState.enemies[0].set_position(glm::vec3(4.0f, 0.0f, 0.0f));
    //creating ghost enemey
    std::vector<GLuint> textureIdsGhost = { loadTexture(g_ghost) };
    std::vector<std::vector<int>> animationsGhost = {{0}};
    g_gameState.enemies[1] = Entity(textureIdsGhost, 1.0f, 1.9f, animationsGhost, 0.0f, 1, 0, 1, 1, IDLE, ENEMY, 1.43f, 1.43f);
    g_gameState.enemies[1].set_ai_type(GHOST);
    g_gameState.enemies[1].set_ai_state(IDLING);
    g_gameState.enemies[1].set_animation_state(IDLE);
    g_gameState.enemies[1].set_position(glm::vec3(7.5f, 0.0f, 0.0f));

    //creating slime enemy
    //since slime has no jumping animation I put a 0 for the texture ID and an empty list for the animations
    std::vector<GLuint> textureIdsSlime = { loadTexture(g_slime), 0, loadTexture(g_slimeWalk)};
    std::vector<std::vector<int>> animationsSlime = { {0},{}, { 0, 1 } };
    g_gameState.enemies[2] = Entity(textureIdsSlime, 1.0f, 1.9f, animationsSlime, 0.0f, 1, 0, 1, 1, IDLE, ENEMY, 0.7f, 0.7f);
    g_gameState.enemies[2].set_ai_type(SLIME);
    g_gameState.enemies[2].set_ai_state(IDLING);
    g_gameState.enemies[2].set_animation_state(IDLE);
    g_gameState.enemies[2].set_position(glm::vec3(14.0f, 0.0f, 0.0f));

    //Get background music started
    g_music = Mix_LoadMUS(BGM_FILEPATH);
    Mix_PlayMusic(g_music, LOOP_FOREVER);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 3);

    //do jump sound effect
    g_jumpSound = Mix_LoadWAV("jump.wav");


    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    // Each object has its own unique ID
    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
}

void process_input()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
        {
            g_app_status = TERMINATED;
        }
    }
    //reset movement
    g_gameState.player->reset_movement();
    //get keyboard state
    const Uint8* keyState = SDL_GetKeyboardState(NULL);
    if (keyState[SDL_SCANCODE_LEFT] && !g_won) { //player is moving left
        g_gameState.player->move_left();
        g_gameState.player->set_animation_state(WALK);
    }else if (keyState[SDL_SCANCODE_RIGHT] && !g_won) {//player is moving right
        g_gameState.player->move_right();
        g_gameState.player->set_animation_state(WALK);
    }
    else if (keyState[SDL_SCANCODE_UP] && g_gameState.player->get_collided_bottom() && !g_won) {//player is jumping, check for bottom collision to prevent moon jump
        //play jump sound effect
        Mix_PlayChannel(NEXT_CHNL, g_jumpSound, PLAY_ONCE);
        Mix_VolumeChunk(g_jumpSound, MIX_MAX_VOLUME / 5);
        g_gameState.player->jump();
        g_gameState.player->set_animation_state(JUMP);
    }
    else if (!g_gameState.player->get_collided_bottom()) { //if the player is in the air use jump animation
        g_gameState.player->set_animation_state(JUMP);
    }
    else {
        g_gameState.player->set_animation_state(IDLE);

    }
    //normalize player movement
    if (glm::length(g_gameState.player->get_movement()) > 1.0f)
        g_gameState.player->normalise_movement();
    
}

void update() {
    //delta time calculations
    float newTick = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = newTick - g_prevTicks;
    g_prevTicks = newTick;

    deltaTime += g_time_accumulator;

    if (deltaTime < g_fixed_time_step) {
        g_time_accumulator = deltaTime;
        return;
    }
    while (deltaTime >= g_fixed_time_step) {
        //update inside here using g_fixed_time_step
        g_gameState.player->update(g_fixed_time_step, nullptr, g_gameState.enemies, numEnemies, g_gameState.map);
        //update all enemies
        for (int ind = 0; ind < numEnemies; ind++) {
            g_gameState.enemies[ind].update(g_fixed_time_step,g_gameState.player, nullptr, 0, g_gameState.map);
        }
        deltaTime -= g_fixed_time_step;
    }
    g_time_accumulator = deltaTime;

    //make camera follow player movement
    g_view_matrix = glm::mat4(1.0f);
    g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-g_gameState.player->get_position().x, 0.0f, 0.0f));

}

void render() {
    //need for making camera follow player
    g_shader_program.set_view_matrix(g_view_matrix);

    glClear(GL_COLOR_BUFFER_BIT);
    
    g_shader_program.set_model_matrix(g_model_matrix);

    //render map
    g_gameState.map->render(&g_shader_program);
    //render player
    g_gameState.player->render(&g_shader_program);
    //render all enemies
    for (int ind = 0; ind < numEnemies; ind++) {
        g_gameState.enemies[ind].render(&g_shader_program);
    }

    //if player is not active show losing message
    if (!g_gameState.player->isActive()) {
        draw_text(&g_shader_program, g_fontSheetTextureID, "Y", 0.5f, 0.01f, glm::vec3(g_gameState.player->get_position().x, 1.0f, 0.0f));
        draw_text(&g_shader_program, g_fontSheetTextureID, "o", 0.5f, 0.01f, glm::vec3(g_gameState.player->get_position().x+0.5f, 1.0f, 0.0f));
        draw_text(&g_shader_program, g_fontSheetTextureID, "u", 0.5f, 0.01f, glm::vec3(g_gameState.player->get_position().x+1.0f, 1.0f, 0.0f));
        draw_text(&g_shader_program, g_fontSheetTextureID, " ", 0.5f, 0.01f, glm::vec3(g_gameState.player->get_position().x +1.5f, 1.0f, 0.0f));
        draw_text(&g_shader_program, g_fontSheetTextureID, "l", 0.5f, 0.01f, glm::vec3(g_gameState.player->get_position().x+2.0f, 1.0f, 0.0f));
        draw_text(&g_shader_program, g_fontSheetTextureID, "o", 0.5f, 0.01f, glm::vec3(g_gameState.player->get_position().x+2.5f, 1.0f, 0.0f));
        draw_text(&g_shader_program, g_fontSheetTextureID, "s", 0.5f, 0.01f, glm::vec3(g_gameState.player->get_position().x+3.0f, 1.0f, 0.0f));
        draw_text(&g_shader_program, g_fontSheetTextureID, "t", 0.5f, 0.01f, glm::vec3(g_gameState.player->get_position().x +3.5f, 1.0f, 0.0f));
    }

    //if all enemies are not active show winning message
    bool enemiesDead = true;
    for (int ind = 0; ind < numEnemies; ind++) {
        if (g_gameState.enemies[ind].isActive()) { //if we find any active enemies set enemiesDead equal to false
            enemiesDead = false;
        }
    }
    if (enemiesDead) {
        g_won = true;
    }
    //render each letter seperately
    if (g_won) {
        draw_text(&g_shader_program, g_fontSheetTextureID, "Y", 0.5f, 0.01f, glm::vec3(g_gameState.player->get_position().x, 1.0f, 0.0f));
        draw_text(&g_shader_program, g_fontSheetTextureID, "o", 0.5f, 0.01f, glm::vec3(g_gameState.player->get_position().x + 0.5f, 1.0f, 0.0f));
        draw_text(&g_shader_program, g_fontSheetTextureID, "u", 0.5f, 0.01f, glm::vec3(g_gameState.player->get_position().x + 1.0f, 1.0f, 0.0f));
        draw_text(&g_shader_program, g_fontSheetTextureID, " ", 0.5f, 0.01f, glm::vec3(g_gameState.player->get_position().x + 1.5f, 1.0f, 0.0f));
        draw_text(&g_shader_program, g_fontSheetTextureID, "w", 0.5f, 0.01f, glm::vec3(g_gameState.player->get_position().x + 2.0f, 1.0f, 0.0f));
        draw_text(&g_shader_program, g_fontSheetTextureID, "o", 0.5f, 0.01f, glm::vec3(g_gameState.player->get_position().x + 2.5f, 1.0f, 0.0f));
        draw_text(&g_shader_program, g_fontSheetTextureID, "n", 0.5f, 0.01f, glm::vec3(g_gameState.player->get_position().x + 3.0f, 1.0f, 0.0f));
    }

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() {
    //remove map from heap
    delete g_gameState.map;
    //remove player from heap
    delete g_gameState.player;
    //remove enemies from heap
    delete[] g_gameState.enemies;
    //free up the music
    Mix_FreeChunk(g_jumpSound);
    Mix_FreeMusic(g_music);

    SDL_Quit();
    
}

int main(int argc, char* argv[])
{
    // Initialise our program whatever that means
    initialise();

    while (g_app_status == RUNNING)
    {
        process_input();  // If the player did anything press a button, move the joystick process it
        update();         // Using the game's previous state, and whatever new input we have, update the game's state
        render();         // Once updated, render those changes onto the screen
    }

    shutdown();  // The game is over, so let's perform any shutdown protocols
    return 0;
}