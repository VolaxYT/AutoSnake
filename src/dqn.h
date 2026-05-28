#pragma once
#include "neural_net.h"

struct Experience {
    std::vector<float> state;
    int action;
    float reward;
    std::vector<float> next_state;
    bool done;
    float priority;
};

class PrioritizedReplayBuffer {
public:
    int capacity;
    int position;
    float alpha;
    float epsilon_prio;

    std::vector<Experience> memory;
    std::mt19937 rng{std::random_device{}()};

    PrioritizedReplayBuffer(int cap, float alpha = 0.6f) : capacity(cap), position(0), alpha(alpha), epsilon_prio(1e-5f) {
        memory.reserve(cap);
    }

    void push(const std::vector<float>& s, int a, float r, const std::vector<float>& s2, bool done, float priority = 1.0f) {
        Experience exp{s, a, r, s2, done, priority};
        if((int)memory.size() < capacity)
            memory.push_back(exp);
        else
            memory[position] = exp;
        position = (position + 1) % capacity;
    }

    std::vector<int> sample(int batch_size){
        std::vector<float> probs(memory.size());
        float total = 0.0f;
        for(int i = 0; i < (int)memory.size(); i++){
            probs[i] = std::pow(memory[i].priority + epsilon_prio, alpha);
            total += probs[i];
        }
        for(auto& p : probs) p /= total;

        std::vector<int> indices;
        indices.reserve(batch_size);
        std::discrete_distribution<int> dist(probs.begin(), probs.end());
        for(int i = 0; i < batch_size; i++)
            indices.push_back(dist(rng));

        return indices;
    }

    void update_priority(int idx, float td_error){
        memory[idx].priority = std::abs(td_error) + epsilon_prio;
    }

    int size() const { return (int)memory.size(); }
};

class DQN {
    public:
        NeuralNet net;
        NeuralNet target_net;

        float epsilon;
        float epsilon_min;
        float epsilon_decay;
        float gamma;
        float lr;

        PrioritizedReplayBuffer buffer;
        int buffer_max;
        int batch_size;
        int steps;
        int target_update;

        std::mt19937 rng{std::random_device{}()};

        DQN(): buffer(10000) {
            epsilon = 1.0f;
            epsilon_min = 0.01f;
            epsilon_decay = 0.9998f;
            gamma = 0.99f;
            lr = 0.001f;
            buffer_max = 10000;
            batch_size = 32;
            steps = 0;
            target_update = 200;
            target_net = net;
        }

        void store(const std::vector<float>& s, int a, float r, const std::vector<float>& s2, bool done){
            buffer.push(s, a, r, s2, done, 1.0f);
        }

        int select_action(const std::vector<float>& state){
            std::uniform_real_distribution<float> dist(0.0f, 1.0f);
            if(dist(rng) < epsilon){
                std::uniform_int_distribution<int> action_dist(0, 3);
                return action_dist(rng);
            }
            auto q = net.forward(state);
            return (int)(std::max_element(q.begin(), q.end()) - q.begin());
        }

        void train(){
            if(buffer.size() < batch_size) return;

            auto indices = buffer.sample(batch_size);

            for(int idx : indices){
                auto& exp = buffer.memory[idx];

                auto q_next_net = net.forward(exp.next_state);
                int  best_action = (int)(std::max_element(q_next_net.begin(), q_next_net.end()) - q_next_net.begin());
                auto q_next_target = target_net.forward(exp.next_state);

                float target = exp.reward;
                if(!exp.done)
                    target += gamma * q_next_target[best_action];

                auto q_vals = net.forward(exp.state);
                float td_error = std::abs(q_vals[exp.action] - target);

                net.train(exp.state, exp.action, target, lr);

                buffer.update_priority(idx, td_error);
            }

            epsilon = std::max(epsilon_min, epsilon * epsilon_decay);
            steps++;
            if(steps % target_update == 0)
                target_net = net;
        }

        void save(const std::string& path){
        net.save(path);
        std::ofstream meta(path + ".meta");
        meta << epsilon << "\n" << steps << "\n";
    }

    void load(const std::string& path){
        net.load(path);
        target_net = net;
        std::ifstream meta(path + ".meta");
        if(meta.is_open())
            meta >> epsilon >> steps;
    }
};