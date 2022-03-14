#version 330

uniform sampler2D sampler_tex;
uniform vec4 diffuse_colour;

in vec3 varying_normals;
in vec2 varying_texcoords;
in vec3 varying_positions;

out vec4 fragment_colour;

void main(void)
{

	vec3 pointLightPos = vec3(1000,100,300);
	vec3 light_direction = vec3(100,100,200);

	vec3 N = normalize(varying_normals);

	vec3 P = varying_positions;

	//Point light
	vec3 pointLight = normalize(pointLightPos - P);

	//Directional light
	vec3 directionalLight = normalize(-light_direction);

	float pointLight_intensity = max(0, dot(pointLight, N));
	float directional_intensity = max(0, dot(directionalLight, N));

	vec3 ambient_light = vec3(0.01);

	vec3 tex_colour = texture(sampler_tex, varying_texcoords).rgb;

	//Ambient light
	tex_colour = ambient_light + tex_colour * (directional_intensity + pointLight_intensity);

	fragment_colour = vec4(tex_colour, 1.0);
	
}