#include "IShakeItAnimator.h"

IShakeItAnimator::IShakeItAnimator(u32 ShakeTime, u16 Amp)
{
    mStartTime = 0;
    mTimeToShake = ShakeTime;
    Amplitude = Amp;
    Direction = 1;
}

void IShakeItAnimator::shake(u32 ShakeTime, u16 Amp)
{
    mTimeToShake = ShakeTime;
	mStartTime = 0;
    Amplitude = Amp;
}

void IShakeItAnimator::animateNode(ISceneNode *node, u32 timeMs)
{
    if (!mStartTime)
	{
        mStartTime = timeMs;
		startPosition = node->getPosition();
	}
	u32 passedTime = timeMs - mStartTime;
    if (passedTime > mTimeToShake)
        return;
    float progress = float(passedTime)/10.0f / float(mTimeToShake);
    node->setPosition(vector3df(node->getPosition().X, startPosition.Y+((1.0f-progress)*(float)Amplitude)*Direction, node->getPosition().Z));
    Direction *= -1;
} 