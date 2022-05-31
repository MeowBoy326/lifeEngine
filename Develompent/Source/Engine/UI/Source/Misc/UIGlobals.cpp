#include "LEBuild.h"
#include "Misc/UIGlobals.h"
#include "ImGUI/ImGUIEngine.h"
#include "UIEngine.h"

// -------------
// GLOBALS
// -------------

#if WITH_IMGUI
�ImGUIEngine*			GImGUIEngine = new �ImGUIEngine();
#endif // WITH_IMGUI

�UIEngine*				GUIEngine = new �UIEngine();