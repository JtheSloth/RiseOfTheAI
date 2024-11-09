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
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Entity.h"


// Default constructor
Entity::Entity()
    : m_position(0.0f), m_movement(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_velocity(0.0f), m_acceleration(0.0f), m_model_matrix(1.0f),
    m_speed(0.0f), m_animation_cols(0), m_animation_frames(0), m_animation_index(0),
    m_animation_rows(0), m_animation_indices(nullptr), m_animation_time(0.0f),
    m_current_animation(IDLE)
{
}

// Parameterized constructor
Entity::Entity(std::vector<GLuint> texture_ids, float speed, float jump_power,
    std::vector<std::vector<int>> animations, float animation_time,
    int animation_frames, int animation_index, int animation_cols,
    int animation_rows, Animation animation, EntityType EntityType, float width, float height)
    : m_position(0.0f), m_movement(0.0f),m_velocity(0.0f),m_acceleration(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f),
    m_speed(speed), m_texture_ids(texture_ids), m_animations(animations),
    m_animation_cols(animation_cols), m_animation_frames(animation_frames),
    m_animation_index(animation_index), m_animation_rows(animation_rows),
    m_animation_time(animation_time), m_current_animation(animation), m_jumping_power(jump_power), 
    m_entity_type(EntityType), m_width(width), m_height(height), m_is_jumping(false), m_is_active(true)
{
    set_animation_state(m_current_animation);  // Initialize animation state

}

Entity::~Entity() { }

void Entity::set_animation_state(Animation new_animation)
{
    m_current_animation = new_animation;
    //set frames, cols, and scaling based on current animation
    if (m_entity_type == PLAYER) {
        if (m_current_animation == IDLE) { //standing still
            m_animation_frames = m_animations[IDLE].size();
            m_animation_cols = m_animations[IDLE].size();
            m_animation_rows = 1;
            m_scale = glm::vec3(1.4f, 1.4f, 0.0f);
        }else if (m_current_animation == JUMP) {//jumping
            m_animation_frames = m_animations[JUMP].size();
            m_animation_cols = m_animations[JUMP].size();
            m_animation_rows = 1;
            m_scale = glm::vec3(1.4f, 1.4f, 0.0f);
        }else if (m_current_animation == WALK) { //walking
            m_animation_frames = m_animations[WALK].size();
            m_animation_cols = m_animations[WALK].size();
            m_animation_rows = 1;
            m_scale = glm::vec3(1.45f, 1.45f, 0.0f);
        }
        
    }
    else if(m_entity_type == ENEMY && m_ai_type == FROG){ //if the entity is a frog enemy
        if (m_current_animation == IDLE) { //standing still
            m_animation_frames = m_animations[IDLE].size();
            m_animation_cols = m_animations[IDLE].size();
            m_animation_rows = 1;
           m_scale = glm::vec3(0.81f, 0.81f, 0.0f);
        }
        else if (m_current_animation == JUMP) {//jumping
            m_animation_frames = m_animations[JUMP].size();
            m_animation_cols = m_animations[JUMP].size();
            m_animation_rows = 1;
            m_scale = glm::vec3(0.88f, 0.88f, 0.0f);
        }
    }
    else if (m_entity_type == ENEMY && m_ai_type == GHOST) { //if the entity is a ghost enemy
        if (m_current_animation == IDLE) { //standing still
            m_animation_frames = m_animations[IDLE].size();
            m_animation_cols = m_animations[IDLE].size();
            m_animation_rows = 1;
            m_scale = glm::vec3(1.43f, 1.43f, 0.0f);
        }
    }
    else if (m_entity_type == ENEMY && m_ai_type == SLIME) {//if the entity is a slime enemy
        if (m_current_animation == IDLE) {//standing still
            m_animation_frames = m_animations[IDLE].size();
            m_animation_cols = m_animations[IDLE].size();
            m_animation_rows = 1;
            m_scale = glm::vec3(0.7f, 0.7f, 0.0f);
        }
        else if (m_current_animation == WALK) {//walking
            m_animation_frames = m_animations[WALK].size();
            m_animation_cols = m_animations[WALK].size();
            m_animation_rows = 1;
            m_scale = glm::vec3(0.7f, 0.7f, 0.0f);
        }

    }
    
    // Update the texture and animation indices based on the current animation
    m_animation_indices = m_animations[m_current_animation].data();
}
void Entity::move_left() {
    m_movement.x -= 1.0f;
    m_movingRight = false;
}
void Entity::move_right() {
    m_movement.x += 1.0f;
    m_movingRight = true;

}
void Entity::reset_movement() {
    m_movement.y = 0;
    m_movement.x = 0;
};
void Entity::ai_activate(Entity* player) {
    //call the correct ai function based on the type of enemy
    if (m_ai_type == FROG) {
        ai_jump(player);
    }
    if (m_ai_type == GHOST) {
        ai_disappearing(player);
    }
    if (m_ai_type == SLIME) {
        ai_guard(player);
    }
}
//function for frog enemy
void Entity::ai_jump(Entity* player) {
    //if the player is not active then make ai state IDLING
    if (!player->m_is_active) {
        m_ai_state = IDLING;
        set_animation_state(IDLE);
        m_movement = glm::vec3(0.0f, 0.0f, 0.0f);
    }else if (m_ai_state == IDLING) {//if enemy is currently idle check how far they are from player
        if (glm::distance(m_position, player->get_position()) < 3.0f) {
            m_ai_state = JUMPGUARDING;
        }
    }
    else if (m_ai_state == JUMPGUARDING) {//if we are jumping towards player
        if (m_collided_bottom == true) {//if we are touching the bottom then jump and set our movement vector
            set_animation_state(IDLE);
            jump();
            if (m_position.x > player->get_position().x) {
                m_movement = glm::vec3(-0.5f, 0.0f, 0.0f);
                m_movingRight = true;
            }
            else {
                m_movement = glm::vec3(0.5f, 0.0f, 0.0f);
                m_movingRight = false;
            }
        }
        if (m_collided_bottom == false) { // if in the air use jump animation
            set_animation_state(JUMP);
        }
        if (glm::distance(m_position, player->get_position()) > 3.0f) { //if player is far enough switch to IDLING
            m_ai_state = IDLING;
            m_movement = glm::vec3(0.0f, 0.0f, 0.0f);
            set_animation_state(IDLE);
        }
    }
}
//function for ghost enemey
void Entity::ai_disappearing(Entity* player) {
    //if player is not active make ghost appear and return
    if (!player->m_is_active) {
        m_disappear = false;
        m_ai_state = IDLING;
        return;
    }
    //disappear are reappear based on current counter value 
    if (m_disappear_counter >= 300 ) {
        m_disappear_counter = 0;
        m_disappear = !m_disappear;
    }
    m_disappear_counter++;
}
//function for slime enemy
void Entity::ai_guard(Entity* player) {
    //if the player is not active then make ai state IDLING
    if (!player->m_is_active) {
        m_ai_state = IDLING;
        set_animation_state(IDLE);
        m_movement = glm::vec3(0.0f, 0.0f, 0.0f);
    }
    else if (m_ai_state == IDLING) {//if enemy is currently idle check how far they are from player
        if (glm::distance(m_position, player->get_position()) < 3.0f) {
            m_ai_state = GUARDING;
        }
    }
    else if (m_ai_state == GUARDING) {//if we are moving towards player
        //move depending on which way the player is going
        if (m_position.x > player->get_position().x) {
            m_movement = glm::vec3(-0.5f, 0.0f, 0.0f);
            m_movingRight = true;
            set_animation_state(WALK);
        }
        else {
            m_movement = glm::vec3(0.5f, 0.0f, 0.0f);
            m_movingRight = false;
            set_animation_state(WALK);
        }
        if (glm::distance(m_position, player->get_position()) > 3.0f) { //if player is far enough switch to IDLING
            m_ai_state = IDLING;
            m_movement = glm::vec3(0.0f, 0.0f, 0.0f);
            set_animation_state(IDLE);
        }
    }
}

// Render the appropriate texture and animation frame
void Entity::draw_sprite_from_texture_atlas(ShaderProgram* program)
{
    GLuint current_texture = m_texture_ids[m_current_animation];  // Get the right texture

    float u_coord = (float)(m_animation_index % m_animation_cols) / (float)m_animation_cols;
    float v_coord = (float)(m_animation_index / m_animation_cols) / (float)m_animation_rows;

    float width = 1.0f / (float)m_animation_cols;
    float height = 1.0f / (float)m_animation_rows;

    float tex_coords[] =
    {
        u_coord, v_coord + height, u_coord + width, v_coord + height, u_coord + width,
        v_coord, u_coord, v_coord + height, u_coord + width, v_coord, u_coord, v_coord
    };

    float vertices[] =
    {
        -0.5, -0.5, 0.5, -0.5,  0.5, 0.5,
        -0.5, -0.5, 0.5,  0.5, -0.5, 0.5
    };

    glBindTexture(GL_TEXTURE_2D, current_texture);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0,
        vertices);
    glEnableVertexAttribArray(program->get_position_attribute());

    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0,
        tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

void Entity::update(float delta_time, Entity* player, Entity* collidable_entities, int collidable_entity_count, Map* map)
{
    //if we are not active just return
    if (!m_is_active) {
        return;
    }
    //if we are an enemy activate ai
    if (m_entity_type == ENEMY) {
        ai_activate(player);
    }
    
    m_animation_time += delta_time;
    float frames_per_second = 1.0f / SECONDS_PER_FRAME;

    m_collided_top = false;
    m_collided_bottom = false;
    m_collided_left = false;
    m_collided_right = false;

    if (m_animation_time >= frames_per_second)
    {
        m_animation_time = 0.0f;
        m_animation_index++;

        if (m_animation_index >= m_animation_frames)
        {
            m_animation_index = 0;
        }
    }
    //gravity
    m_acceleration.y = -1.98f;

    m_velocity.x = m_movement.x * m_speed;
    m_velocity += m_acceleration * delta_time;

    if (m_is_jumping)
    {
        m_is_jumping = false;
        m_velocity.y += m_jumping_power;
    }
    

    m_position.y += m_velocity.y * delta_time;
    check_collision_y(collidable_entities, collidable_entity_count);
    check_collision_y(map);

    m_position.x += m_velocity.x * delta_time;
    check_collision_x(collidable_entities, collidable_entity_count);
    check_collision_x(map);

    

    m_model_matrix = glm::mat4(1.0f);
    m_model_matrix = glm::translate(m_model_matrix, m_position);

    //if moving left flip enitity over
    if (!m_movingRight) {
        m_model_matrix = glm::rotate(m_model_matrix, 135.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    }
    m_model_matrix = glm::scale(m_model_matrix, m_scale);
   
}

void Entity::render(ShaderProgram* program)
{
    //don't render player if they aren't active or if they are disappearing
    if (!m_is_active || m_disappear) {
        return;
    }
    program->set_model_matrix(m_model_matrix);

    if (m_animation_indices != nullptr) draw_sprite_from_texture_atlas(program);
    if (m_animation_indices == nullptr) {
        std::cout << "ERROR";
    }
}

// ————— COLLISIONS ————— //
bool const Entity::check_collision(Entity* other) const
{
    float x_distance = fabs(m_position.x - other->m_position.x) - ((m_width + other->m_width) / 2.0f);
    float y_distance = fabs(m_position.y - other->m_position.y) - ((m_height + other->m_height) / 2.0f);

    return x_distance < 0.0f && y_distance < 0.0f;
}

void const Entity::check_collision_y(Entity* collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Entity* collidable_entity = &collidable_entities[i];

        if (collidable_entity->m_is_active && check_collision(collidable_entity) && !collidable_entity->m_disappear)
        {
            float y_distance = fabs(m_position.y - collidable_entity->m_position.y);
            float y_overlap = fabs(y_distance - (m_height / 2.0f) - (collidable_entity->m_height / 2.0f));
            if (m_velocity.y > 0)
            {
                //m_position.y -= y_overlap;
                m_velocity.y = 0;

                // Collision!
                m_collided_top = true;
                //if the player had a top collision with an enemy deactivate them
                deactivate();

            }
            else if (m_velocity.y < 0)
            {
                //m_position.y += y_overlap;
                m_velocity.y = 0;

                // Collision!
                m_collided_bottom = true;
                //since you have hit the top of an enemy make the enemy inactive
                collidable_entity->deactivate();
            }
        }
    }
}

void const Entity::check_collision_x(Entity* collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Entity* collidable_entity = &collidable_entities[i];

        if (collidable_entity->m_is_active && check_collision(collidable_entity) && !collidable_entity->m_disappear)
        {
            float x_distance = fabs(m_position.x - collidable_entity->m_position.x);
            float x_overlap = fabs(x_distance - (m_width / 2.0f) - (collidable_entity->m_width / 2.0f));
            if (m_velocity.x > 0)
            {
                //m_position.x -= x_overlap;
                m_velocity.x = 0;

                // Collision!
                m_collided_right = true;
                //if a player had a right collision with an enemy deactivate them
                deactivate();


            }
            else if (m_velocity.x < 0)
            {
                //m_position.x += x_overlap;
                m_velocity.x = 0;

                // Collision!
                m_collided_left = true;
                //if a player had a left collision with an enemy deactivate them
                deactivate();

            }
        }
    }
}

void const Entity::check_collision_y(Map* map)
{
    // Probes for tiles above
    glm::vec3 top = glm::vec3(m_position.x, m_position.y + (m_height / 2), m_position.z);
    glm::vec3 top_left = glm::vec3(m_position.x - (m_width / 2), m_position.y + (m_height / 2), m_position.z);
    glm::vec3 top_right = glm::vec3(m_position.x + (m_width / 2), m_position.y + (m_height / 2), m_position.z);

    // Probes for tiles below
    glm::vec3 bottom = glm::vec3(m_position.x, m_position.y - (m_height / 2), m_position.z);
    glm::vec3 bottom_left = glm::vec3(m_position.x - (m_width / 2), m_position.y - (m_height / 2), m_position.z);
    glm::vec3 bottom_right = glm::vec3(m_position.x + (m_width / 2), m_position.y - (m_height / 2), m_position.z);

    float penetration_x = 0;
    float penetration_y = 0;

    // If the map is solid, check the top three points
    if (map->is_solid(top, &penetration_x, &penetration_y) && m_velocity.y > 0)
    {
        m_position.y -= penetration_y;
        m_velocity.y = 0;
        m_collided_top = true;
    }
    else if (map->is_solid(top_left, &penetration_x, &penetration_y) && m_velocity.y > 0)
    {
        m_position.y -= penetration_y;
        m_velocity.y = 0;
        m_collided_top = true;
    }
    else if (map->is_solid(top_right, &penetration_x, &penetration_y) && m_velocity.y > 0)
    {
        m_position.y -= penetration_y;
        m_velocity.y = 0;
        m_collided_top = true;
    }

    // And the bottom three points
    if (map->is_solid(bottom, &penetration_x, &penetration_y) && m_velocity.y < 0)
    {
        m_position.y += penetration_y;
        m_velocity.y = 0;
        m_collided_bottom = true;
    }
    else if (map->is_solid(bottom_left, &penetration_x, &penetration_y) && m_velocity.y < 0)
    {
        m_position.y += penetration_y;
        m_velocity.y = 0;
        m_collided_bottom = true;
    }
    else if (map->is_solid(bottom_right, &penetration_x, &penetration_y) && m_velocity.y < 0)
    {
        m_position.y += penetration_y;
        m_velocity.y = 0;
        m_collided_bottom = true;

    }
}

void const Entity::check_collision_x(Map* map)
{
    // Probes for tiles; the x-checking is much simpler
    glm::vec3 left = glm::vec3(m_position.x - (m_width / 2), m_position.y, m_position.z);
    glm::vec3 right = glm::vec3(m_position.x + (m_width / 2), m_position.y, m_position.z);

    float penetration_x = 0;
    float penetration_y = 0;

    if (map->is_solid(left, &penetration_x, &penetration_y) && m_velocity.x < 0)
    {
        m_position.x += penetration_x;
        m_velocity.x = 0;
        m_collided_left = true;
    }
    if (map->is_solid(right, &penetration_x, &penetration_y) && m_velocity.x > 0)
    {
        m_position.x -= penetration_x;
        m_velocity.x = 0;
        m_collided_right = true;
    }
}
// ————— GETTERS ————— //
glm::vec3 const Entity::get_position() const { return m_position; }
glm::vec3 const Entity::get_velocity()     const { return m_velocity; }
glm::vec3 const Entity::get_acceleration() const { return m_acceleration; }
glm::vec3 const Entity::get_movement() const { return m_movement; }
glm::vec3 const Entity::get_scale() const { return m_scale; }
float     const Entity::get_speed()        const { return m_speed; }
bool      const Entity::get_collided_top() const { return m_collided_top; }
bool      const Entity::get_collided_bottom() const { return m_collided_bottom; }
bool      const Entity::get_collided_right() const { return m_collided_right; }
bool      const Entity::get_collided_left() const { return m_collided_left; }

// ————— SETTERS ————— //
void const Entity::set_position(glm::vec3 new_position) { m_position = new_position; }
void const Entity::set_velocity(glm::vec3 new_velocity) { m_velocity = new_velocity; }
void const Entity::set_acceleration(glm::vec3 new_acceleration) { m_acceleration = new_acceleration; }
void const Entity::set_movement(glm::vec3 new_movement) { m_movement = new_movement; }
void const Entity::set_scale(glm::vec3 new_scale) { m_scale = new_scale; }
void const Entity::set_speed(float new_speed) { m_speed = new_speed; }
void const Entity::set_jumping_power(float new_jumping_power) { m_jumping_power = new_jumping_power; }
void const Entity::set_width(float new_width) { m_width = new_width; }
void const Entity::set_height(float new_height) { m_height = new_height; }
void const Entity::set_ai_type(AIType new_ai_type) { m_ai_type = new_ai_type; };
void const Entity::set_ai_state(AIState new_state) { m_ai_state = new_state; };