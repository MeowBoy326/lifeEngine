#include "Misc/AudioGlobals.h"
#include "System/AudioEngine.h"

void �AudioEngine::Init()
{
	GAudioDevice.Init();
}

void �AudioEngine::Shutdown()
{
	GAudioDevice.Shutdown();
}