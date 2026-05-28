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
        steps_without_food = 0;
        spawn_food();
    }

    float update(Grid& grid){
        if(dead) return -10.0f;
        if(cases == 0) return 200.0f;

        dir = next_dir;

        Vec2 new_head;
        if(dir == Direction::RIGHT) new_head = {snake[0].x + 1, snake[0].y};
        else if(dir == Direction::LEFT) new_head = {snake[0].x - 1, snake[0].y};
        else if(dir == Direction::UP) new_head = {snake[0].x, snake[0].y - 1};
        else new_head = {snake[0].x, snake[0].y + 1};

        float reward = -0.03f;

        if(new_head.x < 0 || new_head.x >= 20 || new_head.y < 0 || new_head.y >= 20){
            dead = true;
            return -10.0f;
        }

        for(size_t i = 1; i < snake.size(); i++){
            if(new_head == snake[i]){
                dead = true;
                return -10.0f;
            }
        }

        std::deque<Vec2> new_snake = snake;
        new_snake.push_front(new_head);

        if(new_head == food){
            score++;
            cases--;
            steps_without_food = 0;
            
            reward += 20.0f;
            if(cases > 0) spawn_food();
        } else {
            new_snake.pop_back();
        }

        if(steps_without_food > 100)
            reward -= 0.1f * (steps_without_food - 100) / 100.0f;

        float old_dist = std::abs(food.x - snake[0].x) + std::abs(food.y - snake[0].y);
        float new_dist = std::abs(food.x - new_head.x) + std::abs(food.y - new_head.y);

        snake = new_snake;

        reward += (old_dist - new_dist) * 0.1f;

        grid.clear();
        grid.set(food.x, food.y, Cell::FOOD);
        grid.set(snake[0].x, snake[0].y, Cell::SNAKE_HEAD);
        for(size_t i = 1; i < snake.size(); i++)
            grid.set(snake[i].x, snake[i].y, Cell::SNAKE_BODY);

        return reward;
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

    bool is_dangerous(int x, int y){
        if(x < 0 || x >= 20 || y < 0 || y >= 20) return true;
        return is_occupied(x, y);
    }

    bool is_dead() const {
        return dead;
    }

    std::vector<float> get_state(){
        std::vector<float> state;
        Vec2 head = snake[0];

        Vec2 front, left_rel, right_rel;

        if(dir == Direction::RIGHT){
            front = {head.x + 1, head.y};
            left_rel = {head.x, head.y - 1};
            right_rel = {head.x, head.y + 1};
        } else if(dir == Direction::LEFT){
            front = {head.x - 1, head.y};
            left_rel = {head.x, head.y + 1};
            right_rel = {head.x, head.y - 1};
        } else if(dir == Direction::UP){
            front = {head.x, head.y - 1};
            left_rel = {head.x - 1, head.y};
            right_rel = {head.x + 1, head.y};
        } else {
            front = {head.x, head.y + 1};
            left_rel = {head.x + 1, head.y};
            right_rel = {head.x - 1, head.y};
        }

        state.push_back(is_dangerous(front.x, front.y)     ? 1.0f : 0.0f);
        state.push_back(is_dangerous(left_rel.x, left_rel.y)  ? 1.0f : 0.0f);
        state.push_back(is_dangerous(right_rel.x, right_rel.y) ? 1.0f : 0.0f);

        state.push_back(dir == Direction::UP ? 1.0f : 0.0f);
        state.push_back(dir == Direction::DOWN ? 1.0f : 0.0f);
        state.push_back(dir == Direction::LEFT ? 1.0f : 0.0f);
        state.push_back(dir == Direction::RIGHT ? 1.0f : 0.0f);
        
        state.push_back(is_dangerous(head.x, head.y - 1) ? 1.0f : 0.0f);
        state.push_back(is_dangerous(head.x, head.y + 1) ? 1.0f : 0.0f);
        state.push_back(is_dangerous(head.x - 1, head.y) ? 1.0f : 0.0f);
        state.push_back(is_dangerous(head.x + 1, head.y) ? 1.0f : 0.0f);

        state.push_back(food.y < head.y ? 1.0f : 0.0f);
        state.push_back(food.y > head.y ? 1.0f : 0.0f);
        state.push_back(food.x < head.x ? 1.0f : 0.0f);
        state.push_back(food.x > head.x ? 1.0f : 0.0f);

        return state;
    }

private:
    unsigned int score = 0;
    unsigned int cases = 20*20 - 3;
    int steps_without_food = 0;
    bool dead = false;
    Direction dir = Direction::RIGHT;
    Direction next_dir = Direction::RIGHT;

    std::deque<Vec2> snake = {{10, 10}, {9, 10}, {8, 10}};
    Vec2 food = {15,15};

    std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> dist{0, 19};
};