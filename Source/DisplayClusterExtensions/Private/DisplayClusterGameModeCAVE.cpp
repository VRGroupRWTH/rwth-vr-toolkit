#include "DisplayClusterGameModeCAVE.h"
#include "DisplayClusterPawnCAVE.h"

ADisplayClusterGameModeCAVE::ADisplayClusterGameModeCAVE() : Super()
{
	if (!bIsDisplayClusterActive) return;
	DefaultPawnClass = ADisplayClusterPawnCAVE::StaticClass();
}
