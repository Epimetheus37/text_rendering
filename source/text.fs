#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{   
    float f = texture(text, TexCoords).r;
    if(f > 0.9) {
        vec4 sampled = vec4(1.0, 1.0, 1.0, 1.0);
        color = vec4(textColor, 1.0) * sampled;
    } else if (f > 0.0) {
        color = vec4(0.8, 1.0, 0.2, f);
    } else {
        color = vec4(0.0, 0.0, 0.0, f);
    }
}