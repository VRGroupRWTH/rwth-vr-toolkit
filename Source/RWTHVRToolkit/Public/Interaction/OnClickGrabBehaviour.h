#pragma once

#include "CoreMinimal.h"
#include "ClickBehaviour.h"
#include "OnClickGrabBehaviour.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RWTHVRTOOLKIT_API UOnClickGrabBehaviour : public UClickBehaviour
{
	GENERATED_BODY()
	
public:
	UOnClickGrabBehaviour();
	
	virtual void OnClickStart(const UIntenSelectComponent* IntenSelect, const FVector& Point) override;
	virtual void OnClickEnd(const UIntenSelectComponent* IntenSelect) override;

	UPrimitiveComponent* GetFirstComponentSimulatingPhysics(const AActor* TargetActor);

	// recursively goes up the hierarchy and returns the highest parent simulating physics
	UPrimitiveComponent* GetHighestParentSimulatingPhysics(UPrimitiveComponent* Comp);

	UPROPERTY()
		UPrimitiveComponent* MyPhysicsComponent;
};
