#pragma once
#include "config.h"

enum class Direction { UP, DOWN, LEFT, RIGHT };

struct Vec2 {
    int x, y;
    bool operator==(const Vec2& o) const { return x == o.x && y == o.y; }
};

class SnakeGame {
public:
    void reset(){
        score = 0;
        dead = false;
        dir = Direction::RIGHT;
        snake = {{10, 10}, {9, 10}, {8, 10}};
        food;
        cases = 20*20-3;
        spawn_food();
    }

    void update(Grid& grid){
        if(dead) return;
        if(cases == 0) return;

        dir = next_dir;

        std::deque<Vec2> new_snake;
        if(dir == Direction::RIGHT){
            new_snake.push_back({snake[0].x + 1, snake[0].y});
        }else if(dir == Direction::LEFT){
            new_snake.push_back({snake[0].x - 1, snake[0].y});
        }else if(dir == Direction::UP){
            new_snake.push_back({snake[0].x, snake[0].y - 1});
        }else if(dir == Direction::DOWN){
            new_snake.push_back({snake[0].x, snake[0].y + 1});
        }
        for(int i = 0; i < snake.size() - 1; i++){
            new_snake.push_back(snake[i]);
        }

        if(new_snake[0] == food){
            new_snake.push_back(snake[snake.size() - 1]);
            cases -= 1;
            if(cases > 0)
                spawn_food();
        }

        snake = new_snake;

        if(snake[0].x < 0 || snake[0].x >= 20 || snake[0].y < 0 || snake[0].y >= 20){
            dead = true;
            return;
        }

        for(int i = 1; i < snake.size() -1; i++){
            if(snake[0] == snake[i]){
                dead = true;
                return;
            }
        } 

        grid.clear();
        grid.set(food.x, food.y, Cell::FOOD);
        grid.set(snake[0].x, snake[0].y, Cell::SNAKE_HEAD);
        for(int i = snake.size() - 1; i > 0; i--){
            grid.set(snake[i].x, snake[i].y, Cell::SNAKE_BODY);
        }
    }

    void change_direction(Direction direction){
        if((direction == Direction::UP && dir == Direction::DOWN) || (direction == Direction::DOWN && dir == Direction::UP) || (direction == Direction::RIGHT && dir == Direction::LEFT) || (direction == Direction::LEFT && dir == Direction::RIGHT))
            return;
        next_dir = direction;
    }

    void spawn_food(){
        int x, y;
        do {
            x = dist(rng);
            y = dist(rng);
        } while(is_occupied(x, y));
        food = {x, y};
    }

    bool is_occupied(int x, int y){
        for(Vec2& seg : snake)
            if(seg.x == x && seg.y == y) return true;
        return false;
    }

private:
    unsigned int score = 0;
    unsigned int cases = 3;
    bool dead = false;
    Direction dir = Direction::RIGHT;
    Direction next_dir;

    std::deque<Vec2> snake = {{10, 10}, {9, 10}, {8, 10}};
    Vec2 food = {15,15};

    std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> dist{0, 19};
};