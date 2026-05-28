#include <config.h>
#include <grid.h>
#include <snake_game.h>
#include <neural_net.h>
#include <dqn.h>

// Charge et compile un shader depuis un fichier
unsigned int make_module(const std::string& filepath, unsigned int type){
    std::ifstream file(filepath);
    if(!file.is_open())
        throw std::runtime_error("Impossible d'ouvrir : " + filepath);

    std::stringstream ss;
    ss << file.rdbuf();
    std::string src = ss.str();

    // DEBUG
    //std::cerr << "=== Shader lu : " << filepath << " ===" << std::endl;
    //std::cerr << src << std::endl;
    //std::cerr << "=== Fin ===" << std::endl;

    const char* csrc = src.c_str();

    unsigned int mod = glCreateShader(type);
    glShaderSource(mod, 1, &csrc, NULL);
    glCompileShader(mod);

    int ok;
    glGetShaderiv(mod, GL_COMPILE_STATUS, &ok);
    if(!ok){
        char log[1024];
        glGetShaderInfoLog(mod, 1024, NULL, log);
        throw std::runtime_error(std::string("Shader error:\n") + log);
    }
    return mod;
}

// Lie un vertex shader et un fragment shader en un programme GPU
unsigned int make_shader(const std::string& vert, const std::string& frag){
    unsigned int vs = make_module(vert, GL_VERTEX_SHADER);
    unsigned int fs = make_module(frag, GL_FRAGMENT_SHADER);

    unsigned int prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    int ok;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if(!ok){
        char log[1024];
        glGetProgramInfoLog(prog, 1024, NULL, log);
        throw std::runtime_error(std::string("Link error:\n") + log);
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

int main(int argc, char const *argv[])
{
    if(!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    const float W = 480.0f, H = 480.0f;
    GLFWwindow* window = glfwCreateWindow((int)W, (int)H, "AutoSnake", NULL, NULL);

    if(!window) {
        std::cerr << "Erreur : fenêtre GLFW non créée" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwSwapInterval(0);
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    std::string base = std::filesystem::canonical(std::filesystem::path(argv[0]).parent_path()).string();
    unsigned int shader = make_shader(
        base + "/shaders/vertex.glsl",
        base + "/shaders/fragment.glsl"
    );

    glm::mat4 proj = glm::ortho(0.0f, 480.0f, 480.0f, 0.0f);

    glUseProgram(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(proj));

    Grid grid;
    SnakeGame game;
    Direction dirs[] = {Direction::UP, Direction::DOWN, Direction::LEFT, Direction::RIGHT};
    DQN dqn;

    double step_interval = 0.01;
    double last_step = glfwGetTime();
    bool autoplay = false;
    bool render = true;
    int iterations = 1;

    std::string model_path = base + "/model.bin";

    if(std::filesystem::exists(model_path)){
        std::cerr << "Chargement du modèle..." << std::endl;
        dqn.load(model_path);
    }

    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        static int prev_up = GLFW_RELEASE;
        static int prev_down = GLFW_RELEASE;
        static int prev_left = GLFW_RELEASE;
        static int prev_right = GLFW_RELEASE;
        static int prev_reset = GLFW_RELEASE;
        static int prev_t = GLFW_RELEASE;

        int cur_up = glfwGetKey(window, GLFW_KEY_UP);
        int cur_down = glfwGetKey(window, GLFW_KEY_DOWN);
        int cur_left = glfwGetKey(window, GLFW_KEY_LEFT);
        int cur_right = glfwGetKey(window, GLFW_KEY_RIGHT);
        int cur_reset = glfwGetKey(window, GLFW_KEY_R);
        int cur_t = glfwGetKey(window, GLFW_KEY_T);

        if(cur_up == GLFW_PRESS && prev_up == GLFW_RELEASE){
            game.change_direction(Direction::UP);
        }
        else if(cur_down == GLFW_PRESS && prev_down == GLFW_RELEASE){
            game.change_direction(Direction::DOWN);
        }
        else if(cur_right == GLFW_PRESS && prev_right == GLFW_RELEASE){
            game.change_direction(Direction::RIGHT);
        }
        else if(cur_left == GLFW_PRESS && prev_left == GLFW_RELEASE){
            game.change_direction(Direction::LEFT);
        }
        if(cur_reset == GLFW_PRESS && prev_reset == GLFW_RELEASE){
            game.reset();
        }
        if(cur_t == GLFW_PRESS && prev_t == GLFW_RELEASE){
            render = !render;
        }

        prev_up = cur_up;
        prev_down = cur_down;
        prev_left = cur_left;
        prev_right = cur_right;
        prev_reset = cur_reset;
        prev_t = cur_t;

        double now = glfwGetTime();
        if(now - last_step >= step_interval){
            last_step = now;
            if(game.is_dead()){
                iterations++;
                if(iterations % 10 == 0)
                    dqn.save(model_path);
                for(int i = 0; i < 10; i++){
                    auto t1 = std::chrono::high_resolution_clock::now();
                    dqn.train();
                    auto t2 = std::chrono::high_resolution_clock::now();
                    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
                    std::cerr << "train() : " << ms << " ms" << std::endl;
                }

                game.reset();
                grid.clear();
            }else{
                auto state = game.get_state();
                int action  = dqn.select_action(state);
                game.change_direction(dirs[action]);
                float reward = game.update(grid);

                auto next_state = game.get_state();
                bool done = game.is_dead();
                dqn.store(state, action, reward, next_state, done);
            }
        }

        std::string title = "AutoSnake | Iterations: " + std::to_string(iterations) + " | Render: " + std::string(render ? "ON" : "OFF");
        glfwSetWindowTitle(window, title.c_str());

        if(render){
            grid.draw();
            glfwSwapBuffers(window);
        }
    }

    glDeleteProgram(shader);
    glfwTerminate();
    return 0;
}
