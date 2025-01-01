#include <iostream>
#include <string>
#include "raylib.h"
#include "raymath.h"

class Entity
{
public:
    Vector2 position;

    Entity(Vector2 position);

    virtual void update();
    virtual void render();
private:
};

class Player : public Entity
{
public:
    Vector2 velocity;
    float speed;

    Player(Vector2 position, float speed);

    void update() override;
    void render() override;
private:
    float base_speed;

    float dash_duration;
    float dash_time_elapsed;
    float dash_cooldown;
    float dash_speed;
    bool can_dash;

    void handle_basic_movement();
    void dash();
    void reset_dash();
};

int main(void)
{
    const int screenWidth = 1270;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "boss-rush");
    SetTargetFPS(60);

    Player player(Vector2 {screenWidth/2.0f, screenHeight/2.0f}, 40.0f);

    while (!WindowShouldClose())
    {
        player.update();

        BeginDrawing();
        ClearBackground(LIGHTGRAY);

        player.render();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

Entity::Entity(Vector2 position)
    : position(position)
{
}

void Entity::update()
{
}

void Entity::render()
{
    DrawCircleV(position, 10.0, RED);
}

Player::Player(Vector2 position, float base_speed)
    : Entity(position),
    base_speed(base_speed),
    speed(base_speed),
    dash_duration(0.15f), 
    dash_time_elapsed(0.0f),
    dash_speed(base_speed*10.0f),
    dash_cooldown(5.0f),
    can_dash(true)
{
}

void Player::update()
{
    handle_basic_movement();

    if (dash_time_elapsed >= dash_duration) {
        std::cout << "dash reset" << '\n';
        reset_dash();
    }

    if (IsKeyPressed(KEY_SPACE))
        dash();


    if (!can_dash) {
        dash_time_elapsed += GetFrameTime();
    }

    if (velocity != Vector2Zeros) {
        position = Vector2Add(position, velocity);
    }
}

void Player::render()
{
    DrawCircleV(position, 10.0f, RED);
    std::string time_text = "Dash time elapsed:" + std::to_string(dash_time_elapsed);
    std::string speed_text = "Current speed:" + std::to_string(speed);
    DrawText(time_text.c_str(), GetScreenWidth()/2.0f, GetScreenHeight()/2.0f, 20, BLACK);
    DrawText(speed_text.c_str(), GetScreenWidth()/2.0f, GetScreenHeight()/2.0f + 30, 20, BLACK);
}

void Player::handle_basic_movement()
{
    if (IsKeyDown(KEY_W)) {
        velocity.y = -1.0f * speed * GetFrameTime();
    }
    else if (IsKeyDown(KEY_S)) {
        velocity.y = 1.0f * speed * GetFrameTime();
    }
    else {
        velocity.y = 0.0f;
    }

    if (IsKeyDown(KEY_A)) {
        velocity.x = -1.0f * speed * GetFrameTime();
    }
    else if (IsKeyDown(KEY_D)) {
        velocity.x = 1.0f * speed * GetFrameTime();
    }
    else {
        velocity.x = 0.0f;
    }
}

void Player::dash()
{
    if (can_dash) {
        speed = dash_speed;
        can_dash = false;
    }
}

void Player::reset_dash()
{
    dash_time_elapsed = 0.0f;
    speed = base_speed;
    can_dash = true;
}
