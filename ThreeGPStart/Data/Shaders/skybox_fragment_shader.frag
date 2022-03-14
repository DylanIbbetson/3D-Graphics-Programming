#version 330

uniform sampler2D sampler_tex;
uniform vec4 diffuse_colour;

in vec3 varying_normals;
in vec2 varying_texcoords;
in vec3 varying_positions;

out vec4 fragment_colour;

void main(void)
{

	vec3 tex_colour = texture(sampler_tex, varying_texcoords).rgb;
 

	fragment_colour = vec4(tex_colour, 1.0);
	
}