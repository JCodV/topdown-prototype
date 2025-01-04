#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include "raylib.h"
#include "raymath.h"

const int ENEMY_BODY_RADIUS = 10;
const int ENEMY_MAX_HEALTH = 40;
const float ENEMY_BASE_SPEED = 30.0f;
const float ENEMY_BASE_MELEE_DAMAGE = 10.0f;

const int PLAYER_BODY_RADIUS = 15;
const int PLAYER_MAX_HEALTH = 100;
const float PLAYER_BASE_SPEED = 50.0f;
const float PLAYER_BASE_MELEE_DAMAGE = 10.0f;

// 8 x 6
const std::vector<std::vector<int>> level_one_map =
    {
        {1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1},
    };

const std::vector<std::vector<int>> level_two_map;

const std::vector<std::vector<int>> level_three_map;

class Entity
{
public:
    Vector2 position;

    Entity();
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

class Obstacle : public Entity
{
public:
    int width;
    int height;
    bool is_destructable;

    Obstacle(Vector2 position, int width, int height, bool is_destructable);

    void update() override;
    void render() override;
private:
};

enum class LevelNumber
{
    UNDEFINED,
    ONE,
    TWO,
    THREE,
};

class Level
{
public:
    LevelNumber level_number;
    Player* player;
    std::vector<Enemy*> active_enemies;
    std::vector<Obstacle> active_obstacles;

    Level(LevelNumber level_number);

    void update();
    void render();
private:
    std::vector<std::vector<Entity>> level_map;
    int score;
    bool is_game_over;
    bool is_complete;

    void set_player();
    void spawn_enemies();
    void spawn_obstacles();
    void load_map(const std::vector<std::vector<int>> map_data);

    void cleanup_all();
    // void cleanup_dead_enemies();
    // void cleanup_obstacles();
    // void cleanup_player();
};

enum class GameScreen
{
    UNDEFINED,
    TITLE,
    LOADING,
    OPTIONS,
    GAMEPLAY
};

class Game
{
public:
    GameScreen screen;

    Game();
    void run();
private:
    std::unique_ptr<Level> current_level;

    void update();
    void render();

    void switch_level(LevelNumber next_level);
    void cleanup();
};

int main(void)
{
    const int screenWidth = 1270;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "boss-rush");
    SetTargetFPS(60);

    Level level(LevelNumber::ONE);
    Player* player = new Player(Vector2 {GetScreenWidth()/2.0f, GetScreenHeight()/2.0f}, 100, 50.0f, 10.0f);
    Enemy* enemy = new Enemy(player, Vector2 {GetScreenWidth()/2.0f, GetScreenHeight()/2.0f}, 50, 30.0f, 10.0f);
    while (!WindowShouldClose())
    {
        level.update();
        //player->update();
        //enemy->update();


        BeginDrawing();
        ClearBackground(LIGHTGRAY);
        level.render();

        //player->render();
        //enemy->render();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

Entity::Entity()
    : position(Vector2Zeros)
{
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
    DrawCircleV(position, ENEMY_BODY_RADIUS, RED);
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
    DrawCircleV(position, PLAYER_BODY_RADIUS, RED);
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
    std::cout << "CHOMP" << '\n';
    target->health -= base_melee_damage;
}

Obstacle::Obstacle(Vector2 position, int width, int height, bool is_destructable)
    : Entity(position), width(width), height(height), is_destructable(is_destructable)
{
}

void Obstacle::update()
{
}

void Obstacle::render()
{
    DrawRectangle(int(position.x), int(position.y), width, height, BLUE);
}

Level::Level(LevelNumber level_number)
{
    switch (level_number) {
        case LevelNumber::UNDEFINED:
        {
                std::cerr << "UNDEFINED LEVEL NUMBER!" << '\n';
                break;
        }
        case LevelNumber::ONE:
        {
                load_map(level_one_map);
                break;
        }
        case LevelNumber::TWO:
        {
                load_map(level_two_map);
                break;
        }
        case LevelNumber::THREE:
        {
                load_map(level_three_map);
                break;
        }
    }
}

void Level::update()
{

    for (Enemy* enemy : active_enemies) {
        enemy->update();

        if (CheckCollisionPointCircle(GetMousePosition(), enemy->position, ENEMY_BODY_RADIUS)) {
            player->target = enemy;
        }
    }

    player->update();

    for (Obstacle& obs : active_obstacles) {
        obs.update();
    }
}

void Level::render()
{
    for (Enemy* enemy : active_enemies) {
        enemy->render();
    }

    player->render();

    for (Obstacle& obs : active_obstacles) {
        obs.render();
    }
}

void Level::set_player()
{

}

void Level::spawn_enemies()
{
    switch (level_number) {
        case LevelNumber::UNDEFINED:
            break;
        case LevelNumber::ONE:
            break;
        case LevelNumber::TWO:
            break;
        case LevelNumber::THREE:
            break;
    }
}

void Level::spawn_obstacles()
{
    switch (level_number) {
        case LevelNumber::UNDEFINED:
            break;
        case LevelNumber::ONE:
            break;
        case LevelNumber::TWO:
            break;
        case LevelNumber::THREE:
            break;
    }
}

void Level::load_map(const std::vector<std::vector<int>> map_data)
{
    // free space = 0
    // walls = 1
    // enemies = 2
    // player = 3


    int tile_width = GetScreenWidth() / map_data.size();
    int tile_height = GetScreenHeight() / map_data[0].size();

    std::vector<std::vector<Entity>> map_to_load(map_data.size(), std::vector<Entity>(map_data[0].size()));
    for (int i = 0; i < map_data.size(); i++) {
        for (int j = 0; j < map_data[i].size(); j++) {
            Vector2 position {(float)tile_width * i, (float)tile_height * j};
            switch (map_data[i][j]) {
                case 0:
                    break;
                case 1:
                {
                    Obstacle wall(position, tile_width, tile_height, false);
                    active_obstacles.push_back(wall);
                    break;
                }
                case 2:
                {
                    Enemy* enemy = new Enemy(nullptr, position, ENEMY_MAX_HEALTH, ENEMY_BASE_SPEED, ENEMY_BASE_MELEE_DAMAGE);
                    active_enemies.push_back(enemy);
                    break;
                }
                case 3:
                {
                    player = new Player(position, PLAYER_MAX_HEALTH, PLAYER_BASE_SPEED, PLAYER_BASE_MELEE_DAMAGE);
                    break;
                }
            }
        }
    }

    if (player == nullptr) {
        std::cerr << "MAP DATA DOES NOT CONTAIN A PLAYER, UNABLE TO LOAD MAP";
    }
    else {
        for (Enemy* e : active_enemies) {
            e->target = player;
        }
        std::cout << "MAP SUCCESSFULLY LOADED" << '\n';
    }
}

void Level::cleanup_all()
{
}

Game::Game()
{
    InitWindow(1280, 720, "My Game");
    SetTargetFPS(60);
}

void Game::run()
{
    update();
    BeginDrawing();
    ClearBackground(WHITE);
    render();
    EndDrawing();
}

void Game::update()
{
    current_level->update();
}

void Game::render()
{
    current_level->render();
}

void Game::switch_level(LevelNumber next_level)
{
    current_level = std::make_unique<Level>(next_level);
}

void Game::cleanup()
{
}
