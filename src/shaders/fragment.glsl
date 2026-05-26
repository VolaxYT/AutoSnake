#version 400 core

in vec3 fragColor;
out vec4 screenColor;

void main(){
    screenColor = vec4(fragColor, 1.0);
}