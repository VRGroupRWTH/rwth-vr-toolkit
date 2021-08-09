#include "Cluster/Calibratio/Calibratio.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"

void FCalibratio::Register()
{
	/* Registering console command */
	CalibratioConsoleCommand = IConsoleManager::Get().RegisterConsoleCommand(TEXT("Calibratio"), TEXT("Spawn one instance of calibratio"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray< FString >&)
	{
		if(UGameplayStatics::GetActorOfClass(GEngine->GetCurrentPlayWorld(), ACalibratioActor::StaticClass()) != nullptr) return;
		GEngine->GetCurrentPlayWorld()->SpawnActor<ACalibratioActor>();
	}));
}

void FCalibratio::Unregister() const
{
	IConsoleManager::Get().UnregisterConsoleObject(CalibratioConsoleCommand);
}