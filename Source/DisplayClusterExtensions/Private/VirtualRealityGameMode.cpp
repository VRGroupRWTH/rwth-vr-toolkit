#include "VirtualRealityGameMode.h"
#include "VirtualRealityPawn.h"

AVirtualRealityGameMode::AVirtualRealityGameMode() : Super()
{
//	if (!bIsDisplayClusterActive) return;
	DefaultPawnClass = AVirtualRealityPawn::StaticClass();
}
