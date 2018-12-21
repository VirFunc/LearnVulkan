#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main() 
{
	vec3 normal = normalize(fragNormal);
	vec3 lightDir = normalize(vec3(1.0,0.0,1.0));
	vec3 lightColor = vec3(0.4,0.3,0.5);
	float shiness = dot(lightDir,normal);
    outColor = vec4(lightColor * shiness*3.0, 1.0);
}