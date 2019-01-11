#include "MxVkComponent.h"

namespace Mixel
{
	MxVkComponent::MxVkComponent()
	{
	}

	void MxVkComponent::setup(const MxVkManager * manager)
	{
		if (mIsReady)
			destroy();
		mManager = manager;
		mIsReady = mIsReady;
	}

	void MxVkComponent::destroy()
	{
		if (!mIsReady)
			return;
		mManager = nullptr;
		mIsReady = false;
	}


	MxVkComponent::~MxVkComponent()
	{
		destroy();
	}
}
