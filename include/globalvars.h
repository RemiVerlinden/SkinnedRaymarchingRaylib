#ifndef GLOBALVARS_H
#define GLOBALVARS_H

struct GlobalVars
{
	float time;
	float frametime;
	int frame;
	bool paused;
};
// Amount of animations
struct SequenceInfo
{
	int total;
	int index;
};

struct ScreenInfo
{
	const int WIDTH = 900;
	const int HEIGHT = 600;
};

#endif // !GLOBALVARS_H
