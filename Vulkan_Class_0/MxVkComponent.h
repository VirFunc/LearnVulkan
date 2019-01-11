#pragma once
#ifndef _MX_VK_COMPONENT_H_
#define _MX_VK_COMPONENT_H_

#include"MxVkManager.h"

namespace Mixel
{
	class MxVkComponent
	{
	protected:
		bool mIsReady;
		const MxVkManager* mManager;
	public:
		MxVkComponent();
		virtual void setup(const MxVkManager* manager);
		virtual void destroy();
		virtual ~MxVkComponent();
	};
}

#endif // !_MX_VK_OBJECT_H_