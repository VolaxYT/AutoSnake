#pragma once
#include "config.h"

enum class Cell { EMPTY, SNAKE_HEAD, SNAKE_BODY, FOOD };

class Grid {
public:
    Grid(){
        for(int i = 0; i < 20; i++)
            for(int j = 0; j < 20; j++)
                cells[i][j] = Cell::EMPTY;

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glBufferData(GL_ARRAY_BUFFER, 20 * 20 * 6 * 5 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    void draw(){
        std::vector<float> verts;
        float cs = 24.0f;

        for(int y = 0; y < 20; y++){
            for(int x = 0; x < 20; x++){
                float r, g, b;
                if (cells[y][x] == Cell::EMPTY) {
                    r = (x + y) % 2 == 0 ? 0.8f : 0.9f;
                    g = (x + y) % 2 == 0 ? 0.8f : 0.9f;
                    b = (x + y) % 2 == 0 ? 0.8f : 0.9f;
                } else if (cells[y][x] == Cell::SNAKE_HEAD) {
                    r = 0.0f; 
                    g = 1.0f; 
                    b = 0.0f;
                } else if (cells[y][x] == Cell::SNAKE_BODY) {
                    r = 0.0f; 
                    g = 0.8f; 
                    b = 0.0f;
                } else if (cells[y][x] == Cell::FOOD) {
                    r = 1.0f; 
                    g = 0.0f;
                    b = 0.0f;
                }

                push_quad(verts, x*cs, y*cs, cs, cs, r, g, b);
            }
        }

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, verts.size() * sizeof(float), verts.data());
        glDrawArrays(GL_TRIANGLES, 0, (int)verts.size() / 5);
        glBindVertexArray(0);
    }

    ~Grid(){
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
private:
    unsigned int VAO, VBO;
    Cell cells[20][20];

    void push_quad(std::vector<float>& v, float x, float y, float w, float h, float r, float g, float b){
        v.insert(v.end(), {
            x, y, r, g, b,
            x+w, y, r, g, b,
            x+w, y+h, r, g, b,

            x, y, r, g, b,
            x+w, y+h, r, g, b,
            x, y+h, r, g, b
        });
    }
};