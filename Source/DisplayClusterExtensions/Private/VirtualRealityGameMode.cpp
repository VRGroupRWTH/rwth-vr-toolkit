#include "VirtualRealityGameMode.h"
#include "VirtualRealityPawn.h"

AVirtualRealityGameMode::AVirtualRealityGameMode() : Super()
{
	DefaultPawnClass = AVirtualRealityPawn::StaticClass();
}
