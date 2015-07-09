#include <irrlicht.h>

using namespace irr;
using namespace scene;
using namespace core;
class IShakeItAnimator : public ISceneNodeAnimator
{
    public:
        IShakeItAnimator(u32 ShakeTime, u16 Amp);
        virtual ~IShakeItAnimator() {};
        virtual void animateNode(ISceneNode *node, u32 timeMs);
		void IShakeItAnimator::shake(u32 timeMs, u16 Amp=10);
    private:
        vector3df startPosition;
        u32 mStartTime;
        u32 mTimeToShake;
        u16 Amplitude;
        s8 Direction;
}; 