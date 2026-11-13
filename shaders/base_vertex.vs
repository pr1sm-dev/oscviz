#version 330 core
layout (location=0) in vec3 pos;

uniform float time;
uniform float tilt;
uniform float amplitude;
uniform float frequency;
uniform float verticalOffset;
uniform float angularOffset;
uniform float bfCenter;
uniform float falloffRatio;
uniform float powerL;
uniform float powerR;
uniform float power;
uniform float maxMelVal;

uniform sampler1D frequencyTexture;

out VS_OUT {
  float brightness;
} vs_out;

void main() {
    float t = time * 0.25;
    float tilt = radians(tilt);
    mat3 rotX = mat3(1, 0, 0,
                     0, cos(tilt), -sin(tilt),
                     0, sin(tilt),  cos(tilt));   
    mat3 rotZ = mat3(cos(t), -sin(t), 0,
                     sin(t),  cos(t), 0,
                     0,       0,      1);

    float melVal = maxMelVal / 2.0 * (pos.z + 1.0) + 78.0;
    float freqVal = 700.0 * (exp(melVal / 1127.0) - 1.0);
    float binIndex = freqVal * (1024.0/48000.0);

    float amp = texture(frequencyTexture, (binIndex + 0.5) / 1024.0).r;
    float boost = 3 * (log(freqVal/bfCenter)) / log(2.0);

    amp += boost;
    if (amp > 0) amp = 0;

    // -40 to 0, need from 0 to 1
    // exp(-(amp/10)*ln(1/2))
    float movementAmount = exp((log(falloffRatio)*-amp/10.0));
    
    vs_out.brightness = movementAmount;

    // TODO: clean this up
    vec3 offset = amplitude* 0.5* abs(sin(frequency*t + angularOffset*pos.x + verticalOffset*pos.z)) * vec3(pos.x,pos.y,pos.z);
    offset += amplitude*movementAmount * vec3(pos.x, pos.y, pos.z);
    if (frequency == 0) {
        //offset += amplitude * movementAmount * vec3(pos.x, pos.y, pos.z);
    }

    vec3 p = rotX * rotZ * (pos + offset);
    p.x *= 1080.0 / 1920.0;                     


    gl_Position = vec4(p*0.8, 1.0);           
}
