#version 330 core
layout (location=0) in vec3 pos;
uniform float time;

out float depth;

void main() {
    float t = time * 0.25;
    float tilt = radians(70.0);
    mat3 rotX = mat3(1, 0, 0,
                     0, cos(tilt), -sin(tilt),
                     0, sin(tilt),  cos(tilt));   
    mat3 rotZ = mat3(cos(t), -sin(t), 0,
                     sin(t),  cos(t), 0,
                     0,       0,      1);       

    vec3 offset = 0.08*sin(24*t + 0*atan(pos.y, pos.x) + 128*pos.z) * vec3(pos.x, pos.y, 0.0f);
    vec3 p = rotX * rotZ * (pos + offset);
    p.x *= 1080.0 / 1920.0;                     

    depth = p.z;

    gl_Position = vec4(p*0.8, 1.0);           
}
