// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LiveLinkRole.h"
#include "Components/ActorComponent.h"
#include "Roles/LiveLinkTransformTypes.h"
#include "LiveLinkTrackingComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RWTHVRTOOLKIT_API ULiveLinkTrackingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	ULiveLinkTrackingComponent();

	/* Set whether nDisplay should disable LiveLink tracking*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDisableLiveLink = false;

	/* Set the transform of the component in world space of in its local reference frame. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	bool bWorldTransform = false;

	/* Set the LiveLink Subject Representation to be used by this pawn. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FLiveLinkSubjectRepresentation SubjectRepresentation;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	USceneComponent* TrackedComponent;

protected:
	/* LiveLink helper function called on tick */
	void EvaluateLivelink() const;

	/* Helper function that applies the LiveLink data to this component. Taken from the LiveLink Transform Controller.
	 */
	void ApplyLiveLinkTransform(const FTransform& Transform, const FLiveLinkTransformStaticData& StaticData) const;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;
};
