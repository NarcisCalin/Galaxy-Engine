#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec2 res;

uniform int glowSize;
uniform float glowStrength;

out vec4 finalColor;

void main()
{
    vec4 texColor = texture(texture0, fragTexCoord);

    vec4 nFrag = vec4(0.0);
float weightSum = 0.0;

for(int dx = -glowSize; dx <= glowSize; dx++){
    for(int dy = -glowSize; dy <= glowSize; dy++){
        vec2 offset = vec2(float(dx), float(dy)) / res;
        float weight = 1.0 - (abs(float(dx)) + abs(float(dy))) / (2.0 * float(glowSize) + 1.0) * 0.5;
        nFrag += texture(texture0, fragTexCoord + offset) * weight;
        weightSum += weight;
    }
}

nFrag /= weightSum;

    finalColor = texColor + nFrag * glowStrength;
}