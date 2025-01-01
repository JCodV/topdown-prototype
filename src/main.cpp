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

class Actor : public Entity
{
public:
    Actor* target;
    Vector2 velocity;
    float speed;
    float base_speed;
    int base_melee_damage;
    int max_health;
    int health;
    bool is_alive;

    Actor(Vector2 position, float base_speed, int max_health, float base_melee_damage);
    Actor(Actor* target, Vector2 position, float base_speed, int max_health, float base_melee_damage);

    // include in all update methods
    void update_position();
private:
};

class Player : public Actor
{
public:
    Player(Vector2 position, int max_health, float base_speed, int base_melee_damage);

    void update() override;
    void render() override;
private:
    float base_speed;
    int base_melee_damage;

    float dash_duration;
    float dash_time_elapsed;
    float dash_cooldown;
    float dash_speed;
    bool can_dash;

    void handle_basic_movement();
    void dash();
    void reset_dash();
};

class Enemy : public Actor
{
public:
    enum class DistanceFromPlayer
    {
        UNDEFINED,
        CLOSE = 15,
        MID = 30,
        FAR = 100,
    };

    Enemy(Actor* target, Vector2 position, int max_health, float base_speed, int base_melee_damage);

    void update() override;
    void render() override;
private:
    DistanceFromPlayer distance_from_player;

    void chase_player();
    void attack_player();
private:
};

int main(void)
{
    const int screenWidth = 1270;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "boss-rush");
    SetTargetFPS(60);

    Player* player = new Player(Vector2 {GetScreenWidth()/2.0f, GetScreenHeight()/2.0f}, 100, 50.0f, 10.0f);
    Enemy* enemy = new Enemy(player, Vector2 {GetScreenWidth()/2.0f, GetScreenHeight()/2.0f}, 50, 30.0f, 10.0f);
    while (!WindowShouldClose())
    {
        player->update();
        enemy->update();


        BeginDrawing();
        ClearBackground(LIGHTGRAY);

        player->render();
        enemy->render();
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

Actor::Actor(Vector2 position, float base_speed, int max_health, float base_melee_damage)
    : Entity(position),
    target(nullptr),
    velocity(Vector2Zeros),
    speed(base_speed),
    base_speed(base_speed),
    max_health(max_health),
    base_melee_damage(base_melee_damage),
    is_alive(true)
{
    std::cout << "CAUTION: target is not initailized, nullptr" << '\n';
}

Actor::Actor(Actor* target, Vector2 position, float base_speed, int max_health, float base_melee_damage)
    : Entity(position),
    target(target),
    velocity(Vector2Zeros),
    speed(base_speed),
    base_speed(base_speed),
    max_health(max_health),
    base_melee_damage(base_melee_damage),
    is_alive(true)
{
}

void Actor::update_position()
{
    if (velocity != Vector2Zeros) {
        position = Vector2Add(position, velocity);
    }
}

Player::Player(Vector2 position, int max_health, float base_speed, int base_melee_damage)
    : Actor(position, base_speed, max_health, base_melee_damage),
    dash_duration(0.15f), 
    dash_time_elapsed(0.0f),
    dash_speed(base_speed*10.0f),
    dash_cooldown(5.0f),
    can_dash(true)
{
    std::cout << "player spawned";
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

    //std::cout << position.x << ' ' << position.y << '\n';
    update_position();
}

void Player::render()
{
    DrawCircleV(position, 10.0f, RED);
    std::string time_text = "Dash time elapsed:" + std::to_string(dash_time_elapsed);
    std::string speed_text = "Current speed:" + std::to_string(speed);
    DrawText(time_text.c_str(), GetScreenWidth()/2.0f, GetScreenHeight()/2.0f, 20, BLACK);
    DrawText(speed_text.c_str(), GetScreenWidth()/2.0f, GetScreenHeight()/2.0f + 30, 20, BLACK);
    //std::cout << "player render";
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

Enemy::Enemy(Actor* target, Vector2 position, int max_health, float base_speed, int base_melee_damage)
    : Actor(target, position, max_health, base_speed, base_melee_damage)
{
}

void Enemy::update()
{
    if (CheckCollisionCircles(target->position, 10.0f, position, 10.0f)) {
        attack_player();
    }

    chase_player();

    update_position();
}
void Enemy::render()
{
    DrawCircleV(position, 10.0f, GREEN);
}

void Enemy::chase_player()
{
    //std::cout << "Player position: " << target->position.x << ", " << target->position.y << '\n';
    Vector2 direction_to_player = Vector2Normalize(Vector2Subtract(target->position, position));
    velocity = Vector2Scale(direction_to_player, speed * GetFrameTime());
}

void Enemy::attack_player()
{
    //std::cout << "CHOMP" << '\n';
    target->health -= base_melee_damage;
}

