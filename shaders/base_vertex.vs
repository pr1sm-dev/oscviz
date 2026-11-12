#version 330 core
layout (location=0) in vec3 pos;
uniform float time;
uniform float tilt;
uniform float amplitude;
uniform float frequency;
uniform float verticalOffset;
uniform float angularOffset;
uniform float powerL;
uniform float powerR;
uniform float power;

uniform sampler1D frequencyTexture;

out float depth;

void main() {
    float t = time * 0.25;
    float tilt = radians(tilt);
    mat3 rotX = mat3(1, 0, 0,
                     0, cos(tilt), -sin(tilt),
                     0, sin(tilt),  cos(tilt));   
    mat3 rotZ = mat3(cos(t), -sin(t), 0,
                     sin(t),  cos(t), 0,
                     0,       0,      1);

    // ASSUMES SPHERE IS SUCH THAT FROM z=-1 to 1
    float rightMix = (pos.z + 1) / 2; // this is from 0 - 1, represents how much right 
    float leftMix = 1 - rightMix; // represents how much right 

    //float loudness = sqrt((leftMix*powerL)*(leftMix*powerL) + (rightMix*powerR)*(rightMix*powerR));
    float loudness = 0.0f;
    if (pos.z < 0) {
        loudness = sqrt(powerL*powerL+exp(40*pos.z)*powerR*powerR);
    } else {
        loudness = sqrt(powerR*powerR + exp(-40*pos.z)*powerL*powerL);
    }

    float index = (pos.z + 1.0) * 64;
    float amp = texture(frequencyTexture, (index/256)).r;

    // -40 to 0, need from 0 to 1
    float movementAmount = (1.0/80.0) * amp + 1;

    //vec3 offset = vec3(0, 0, 0);
    //offset = amplitude * movementAmount * vec3(pos.x, pos.y, pos.z);

    vec3 offset = amplitude*movementAmount*sin(frequency*t + angularOffset*pos.x + verticalOffset*pos.z) * vec3(pos.x, pos.y, pos.z);
    //offset = offset + (1+power)*(0.03*sin(frequency*0.25*t))*vec3(pos.x,pos.y,0.0f);
    //vec3 offset = -loudness * vec3(pos.x, pos.y, 0.0f);

    //vec3 p = rotX * rotZ * (pos + offset);
    vec3 p = rotX * (pos + offset);
    p.x *= 1080.0 / 1920.0;                     

    depth = p.z;

    gl_Position = vec4(p*0.8, 1.0);           
}
