#include "VirtualRealityGameMode.h"
#include "VirtualRealityPawn.h"

AVirtualRealityGameMode::AVirtualRealityGameMode() : Super()
{
  UE_LOG(LogTemp, Warning, TEXT("AVirtualRealityGameMode Constructor started"));
	if (!bIsDisplayClusterActive) return;
	DefaultPawnClass = AVirtualRealityPawn::StaticClass();
  UE_LOG(LogTemp, Warning, TEXT("AVirtualRealityGameMode Constructor finished"));
}
