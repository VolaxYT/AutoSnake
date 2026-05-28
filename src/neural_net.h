#pragma once
#include "config.h"

struct Layer{
    std::vector<float> weights;
    std::vector<float> biases;
    std::vector<float> last_input;
    std::vector<float> last_output;
    std::vector<float> output_cache;
    int in,out;

    Layer(int in_size, int out_size) :  in(in_size), out(out_size){
        std::mt19937 rng{std::random_device{}()};
        std::normal_distribution<float> weight_dist(0.0f, std::sqrt(2.0f / in_size));

        weights.resize(in*out);
        for(float& w : weights) 
            w = weight_dist(rng);
        biases.assign(out, 0.0f);
    }

    std::vector<float> forward(const std::vector<float>& input, bool apply_relu = true){
        last_input = input;
        last_output.assign(out, 0.0f);
        output_cache.resize(out);

        for(int o = 0; o < out; o++){
            last_output[o] = biases[o];

            for(int i = 0; i < in; i++)
                last_output[o] += weights[o * in + i] * input[i];
            output_cache[o] = apply_relu ? std::max(0.0f, last_output[o]) : last_output[o];
        }

        return output_cache;
    }

    std::vector<float> backward(const std::vector<float>& grad_output, float lr, bool apply_relu){
        std::vector<float> grad(out);
        for(int o = 0; o < out; o++){
            grad[o] = grad_output[o];
            if(apply_relu && last_output[o] <= 0.0f)
                grad[o] = 0.0f;
        }

        std::vector<float> grad_input(in, 0.0f);

        for(int i = 0; i < in; i++)
            for(int o = 0; o < out; o++)
                grad_input[i] += weights[o * in + i] * grad[o];

        for(int o = 0; o < out; o++){
            biases[o] -= lr * grad[o];

            for(int i = 0; i < in; i++)
                weights[o * in + i] -= lr * grad[o] * last_input[i];
        }

        return grad_input;
    }


};

class NeuralNet {
public:
    std::vector<Layer> shared;
    std::vector<Layer> value_head;
    std::vector<Layer> advantage_head;

    NeuralNet(){
        shared.emplace_back(15, 128);
        shared.emplace_back(128, 128);

        value_head.emplace_back(128, 64);
        value_head.emplace_back(64, 1);

        advantage_head.emplace_back(128, 64);
        advantage_head.emplace_back(64, 4);
    }

    std::vector<float> forward(const std::vector<float>& input){
        std::vector<float> x = input;
        for(int i = 0; i < (int)shared.size(); i++){
            bool is_last = (i == (int)shared.size() - 1);
            x = shared[i].forward(x, true);
        }

        std::vector<float> v = x;
        for(int i = 0; i < (int)value_head.size(); i++){
            bool is_last = (i == (int)value_head.size() - 1);
            v = value_head[i].forward(v, !is_last);
        }
        float V = v[0];

        std::vector<float> a = x;
        for(int i = 0; i < (int)advantage_head.size(); i++){
            bool is_last = (i == (int)advantage_head.size() - 1);
            a = advantage_head[i].forward(a, !is_last);
        }

        float mean_a = 0.0f;
        for(float ai : a) mean_a += ai;
        mean_a /= a.size();

        std::vector<float> q(4);
        for(int i = 0; i < 4; i++)
            q[i] = V + a[i] - mean_a;

        return q;
    }

    void train(const std::vector<float>& state, int action, float target, float lr){
        auto q_values = forward(state);

        float error = q_values[action] - target;
        float grad_val;
        if(std::abs(error) <= 1.0f)
            grad_val = 2.0f * error;
        else
            grad_val = 2.0f * (error > 0 ? 1.0f : -1.0f);

        std::vector<float> grad_q(4, 0.0f);
        grad_q[action] = grad_val;

        std::vector<float> grad_a(4);
        float n = 4.0f;
        for(int i = 0; i < 4; i++)
            grad_a[i] = grad_q[action] * (i == action ? (1.0f - 1.0f/n) : (-1.0f/n));

        std::vector<float> grad_v(1);
        grad_v[0] = grad_q[action];

        std::vector<float> g_a = grad_a;
        for(int i = (int)advantage_head.size() - 1; i >= 0; i--){
            bool apply_relu = (i < (int)advantage_head.size() - 1);
            g_a = advantage_head[i].backward(g_a, lr, apply_relu);
        }

        std::vector<float> g_v = grad_v;
        for(int i = (int)value_head.size() - 1; i >= 0; i--){
            bool apply_relu = (i < (int)value_head.size() - 1);
            g_v = value_head[i].backward(g_v, lr, apply_relu);
        }

        std::vector<float> grad_shared(128, 0.0f);
        for(int i = 0; i < 128; i++)
            grad_shared[i] = g_a[i] + g_v[i];

        std::vector<float> g = grad_shared;
        for(int i = (int)shared.size() - 1; i >= 0; i--)
            g = shared[i].backward(g, lr, true);
    }

    void save(const std::string& path){
        std::ofstream file(path, std::ios::binary);
        if(!file.is_open())
            throw std::runtime_error("Impossible d'ouvrir : " + path);

        auto save_layers = [&](std::vector<Layer>& layers){
            for(auto& l : layers){
                file.write((char*)l.weights.data(), l.weights.size() * sizeof(float));
                file.write((char*)l.biases.data(), l.biases.size() * sizeof(float));
            }
        };

        save_layers(shared);
        save_layers(value_head);
        save_layers(advantage_head);

        std::cerr << "Modele sauvegarde : " << path << std::endl;
    }

    void load(const std::string& path){
        std::ifstream file(path, std::ios::binary);
        if(!file.is_open())
            throw std::runtime_error("Impossible d'ouvrir : " + path);

        auto load_layers = [&](std::vector<Layer>& layers){
            for(auto& l : layers){
                file.read((char*)l.weights.data(), l.weights.size() * sizeof(float));
                file.read((char*)l.biases.data(), l.biases.size() * sizeof(float));
            }
        };

        load_layers(shared);
        load_layers(value_head);
        load_layers(advantage_head);

        std::cerr << "Modele charge : " << path << std::endl;
    }
};