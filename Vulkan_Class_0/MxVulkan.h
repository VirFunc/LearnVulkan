#pragma once
#ifndef _MX_VULKAN_H_
#define _MX_VULKAN_H_

#include<vulkan/vulkan.h>

#define SDL_MAIN_HANDLED
#include<SDL2/SDL.h>
#include<SDL2/SDL_vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>

#include<stb/stb_image.h>

#include<iostream>
#include<string>
#include<vector>
#include<chrono>

namespace Mixel
{
	class MxVulkan
	{
	private:


	public:
		MxVulkan();
		~MxVulkan();

	};
}

#endif