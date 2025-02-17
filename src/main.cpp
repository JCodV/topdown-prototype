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
const Color ENEMY_COLOR = GREEN;

const int PLAYER_BODY_RADIUS = 15;
const int PLAYER_MAX_HEALTH = 100;
const float PLAYER_BASE_SPEED = 50.0f;
const float PLAYER_BASE_MELEE_DAMAGE = 10.0f;
const Color PLAYER_COLOR = RED;

const Color SOLID_OBSTACLE_COLOR = BLUE;

const int STAR_RADIUS = 5;
const Color STAR_COLOR = YELLOW;

// 8 x 6
const std::vector<std::vector<int>> level_one_map =
{
        {1, 1, 1, 1, 1, 1, 1, 1},
        {1, 2, 0, 2, 0, 0, 1, 1},
        {1, 0, 0, 0, 0, 2, 0, 1},
        {1, 0, 1, 3, 0, 0, 1, 1},
        {1, 0, 0, 2, 0, 0, 0, 1},
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

class Star : public Entity
{
public:
    bool is_collected;

    Star(Vector2 position);

    void update() override;
    void render() override;
private:
};

class Actor : public Entity
{
public:
    Actor *target;
    Vector2 velocity;
    float speed;
    float base_speed;
    int base_melee_damage;
    int max_health;
    int health;
    bool is_alive;

    Actor(Vector2 position, float base_speed, int max_health, float base_melee_damage);
    Actor(Actor *target, Vector2 position, float base_speed, int max_health, float base_melee_damage);

    // include in all update methods
    void update_position();
private:
    void static_collision_response(); // use case: collision

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

    Enemy(Actor *target, Vector2 position, int max_health, float base_speed, int base_melee_damage);

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
    Player *player;
    std::vector<Enemy*> active_enemies;
    std::vector<Obstacle> active_obstacles;
    std::vector<Star> active_stars;

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
    void handle_entity_obstacle_collisions();

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
    // void cleanup();
};

int main(void)
{
    InitWindow(1280, 720, "Robot game");
    SetTargetFPS(60);
    //bool paused = false;

    Level level(LevelNumber::ONE);

    while (!WindowShouldClose())
    {
        //if (IsKeyPressed(KEY_ENTER)) paused = true;
        //else if (paused && IsKeyPressed(KEY_ENTER)) paused = false;
        //std::cout << "paused: " << paused << '\n';

        //if (!paused) {
            level.update();

            BeginDrawing();
            ClearBackground(LIGHTGRAY);
            level.render();

            EndDrawing();
        //}

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

Star::Star(Vector2 position)
    : Entity(position), is_collected(false)
{
}

void Star::update()
{

}

void Star::render()
{
    DrawCircleV(position, STAR_RADIUS, YELLOW);
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

Actor::Actor(Actor *target, Vector2 position, float base_speed, int max_health, float base_melee_damage)
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
    if (velocity != Vector2Zeros)
    {
        position = Vector2Add(position, velocity);
    }
}

// update later
void Actor::static_collision_response() {
    speed = 0.0f;
}

Player::Player(Vector2 position, int max_health, float base_speed, int base_melee_damage)
    : Actor(position, base_speed, max_health, base_melee_damage),
      dash_duration(0.15f),
      dash_time_elapsed(0.0f),
      dash_speed(base_speed * 10.0f),
      dash_cooldown(5.0f),
      can_dash(true)
{
    std::cout << "player spawned";
}

void Player::update()
{
    handle_basic_movement();

    if (dash_time_elapsed >= dash_duration)
    {
        std::cout << "dash reset" << '\n';
        reset_dash();
    }

    if (IsKeyPressed(KEY_SPACE))
        dash();

    if (!can_dash)
    {
        dash_time_elapsed += GetFrameTime();
    }

    // std::cout << position.x << ' ' << position.y << '\n';
    update_position();
}

void Player::render()
{
    DrawCircleV(position, PLAYER_BODY_RADIUS, RED);
    std::string time_text = "Dash time elapsed:" + std::to_string(dash_time_elapsed);
    std::string speed_text = "Current speed:" + std::to_string(speed);

    DrawText(time_text.c_str(), GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f, 20, BLACK);
    DrawText(speed_text.c_str(), GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f + 30, 20, BLACK);
}

void Player::handle_basic_movement()
{
    if (IsKeyDown(KEY_W))
    {
        velocity.y = -1.0f * speed * GetFrameTime();
    }
    else if (IsKeyDown(KEY_S))
    {
        velocity.y = 1.0f * speed * GetFrameTime();
    }
    else
    {
        velocity.y = 0.0f;
    }

    if (IsKeyDown(KEY_A))
    {
        velocity.x = -1.0f * speed * GetFrameTime();
    }
    else if (IsKeyDown(KEY_D))
    {
        velocity.x = 1.0f * speed * GetFrameTime();
    }
    else
    {
        velocity.x = 0.0f;
    }
}

void Player::dash()
{
    if (can_dash)
    {
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

Enemy::Enemy(Actor *target, Vector2 position, int max_health, float base_speed, int base_melee_damage)
    : Actor(target, position, max_health, base_speed, base_melee_damage)
{
}

void Enemy::update()
{
    if (CheckCollisionCircles(target->position, PLAYER_BODY_RADIUS, position, ENEMY_BODY_RADIUS))
    {
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
    // std::cout << "Player position: " << target->position.x << ", " << target->position.y << '\n';
    Vector2 direction_to_player = Vector2Normalize(Vector2Subtract(target->position, position));
    velocity = Vector2Scale(direction_to_player, speed * GetFrameTime());
}

void Enemy::attack_player()
{
    // std::cout << "CHOMP" << '\n';
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
    DrawRectangle(int(position.x), int(position.y), width, height, SOLID_OBSTACLE_COLOR);
}

Level::Level(LevelNumber level_number)
{
    switch (level_number)
    {
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
    for (Enemy *enemy : active_enemies)
    {
        enemy->update();
        if (CheckCollisionPointCircle(GetMousePosition(), enemy->position, ENEMY_BODY_RADIUS))
        {
            player->target = enemy;
        }
    }

    player->update();

    for (Obstacle &obs : active_obstacles)
    {
        obs.update();
    }

    handle_entity_obstacle_collisions();
}

void Level::render()
{
    for (Enemy *enemy : active_enemies)
    {
        enemy->render();
    }

    player->render();

    for (Obstacle &obs : active_obstacles)
    {
        obs.render();
    }
}

void Level::set_player()
{
}

void Level::spawn_enemies()
{
    switch (level_number)
    {
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
    switch (level_number)
    {
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
    // star = 4

    int tile_width = GetScreenWidth() / map_data.size();
    int tile_height = GetScreenHeight() / map_data[0].size();

    std::vector<std::vector<Entity>> map_to_load(map_data.size(), std::vector<Entity>(map_data[0].size()));
    for (int i = 0; i < map_data.size(); i++)
    {
        for (int j = 0; j < map_data[i].size(); j++)
        {
            Vector2 position{(float)tile_width * i, (float)tile_height * j};
            switch (map_data[i][j])
            {
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
                    Enemy *enemy = new Enemy(nullptr, position, ENEMY_MAX_HEALTH, ENEMY_BASE_SPEED, ENEMY_BASE_MELEE_DAMAGE);
                    active_enemies.push_back(enemy);
                    break;
                }
                case 3:
                {
                    player = new Player(position, PLAYER_MAX_HEALTH, PLAYER_BASE_SPEED, PLAYER_BASE_MELEE_DAMAGE);
                    break;
                }
                case 4:
                {
                    Star star(position);
                    active_stars.push_back(star);
                    break;
                }
            }
        }
    }

    if (player == nullptr)
    {
        std::cerr << "MAP DATA DOES NOT CONTAIN A PLAYER, UNABLE TO LOAD MAP";
    }
    else
    {
        for (Enemy *e : active_enemies)
        {
            e->target = player;
        }
        std::cout << "MAP SUCCESSFULLY LOADED" << '\n';
    }
}

void Level::handle_entity_obstacle_collisions()
{
    for (Obstacle &obs : active_obstacles) {
        Rectangle obs_body_rec { obs.position.x, obs.position.y, float(obs.width), float(obs.height)};

        if (CheckCollisionCircleRec(player->position, PLAYER_BODY_RADIUS, obs_body_rec)) {
            // std::cout << "workss!";
        }

        for (Enemy* en: active_enemies) {
            if (CheckCollisionCircleRec(en->position, ENEMY_BODY_RADIUS, obs_body_rec)) {
                std::cout << "workss!";
            }
        }
    }
}

void Level::cleanup_all()
{
    active_obstacles.clear();

    for (Enemy* e : active_enemies) {
        delete e;
        e = nullptr;
    }
    active_enemies.clear();

    delete player;
    player = nullptr;
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
