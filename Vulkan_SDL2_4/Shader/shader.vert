#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObj
{
	mat4 model;
	mat4 view;
	mat4 proj;
	mat4 normalMat;
}Matrix;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec2 fragTexCoord;

void main() 
{
    gl_Position = Matrix.proj * Matrix.view * Matrix.model * vec4(inPosition, 1.0);
    fragNormal = mat3(Matrix.normalMat) * inNormal;
	fragTexCoord = inTexCoord;
}