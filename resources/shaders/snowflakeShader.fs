#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main()
{
    vec4 texColor = texture(texture1, TexCoords);
    //Here we check if the sampled texture color contains an alpha value lower than a threshold of 0.1 and if so, discard the fragment
    if(texColor.a < 0.1)
        discard;
    FragColor = texColor;
}
