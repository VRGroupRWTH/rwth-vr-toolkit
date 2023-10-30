#pragma once

#include "CoreMinimal.h"
#include "HAL/IConsoleManager.h"
#include "Cluster/IDisplayClusterClusterManager.h"
#include "ClusterConsole.generated.h"

/**
 * The ClusterConsole provides the console command "ClusterExecute"
 * The code catches your command, broadcasts it to every nDisplay node and executes it everywhere
 * 
 * This class has to be registered and unregistered. This can easily be done in every StartupModule/ShutdownModule functions.
 */
USTRUCT()
struct RWTHVRCLUSTER_API FClusterConsole
{
	GENERATED_BODY()
private:
	/* Used for ClusterExecute console command */
	IConsoleCommand* ClusterConsoleCommand = nullptr;
	FOnClusterEventJsonListener ClusterEventListenerDelegate;
	
public:
	void Register();
	void Unregister() const;
};
