#ifndef DEMOSTRUCTS
#define DEMOSTRUCTS

namespace DQ
{
	struct UpdateContext 
	{
		float timestep;
		int frame;
		float animationTime;		// Time-based animation progress
		bool resetAnimation;		// Flag to reset animation to frame 0
	};
}

#endif // !DEMOSTRUCTS
