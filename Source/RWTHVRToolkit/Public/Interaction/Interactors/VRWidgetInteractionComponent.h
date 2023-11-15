// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetInteractionComponent.h"
#include "Pawn/InputExtensionInterface.h"
#include "VRWidgetInteractionComponent.generated.h"

UENUM()
enum EInteractionRayVisibility
{
	Visible UMETA(DisplayName = "Interaction ray visible"),
	VisibleOnHoverOnly UMETA(
		DisplayName =
		"Interaction ray only visible when hovering over interactable world UI widgets"),
	Invisible UMETA(DisplayName = "Interaction ray invisible")
};


UCLASS(Blueprintable, Abstract, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RWTHVRTOOLKIT_API UVRWidgetInteractionComponent : public UWidgetInteractionComponent,
                                                        public IInputExtensionInterface
{
	GENERATED_BODY()

public:
	UVRWidgetInteractionComponent();

	virtual void SetupPlayerInput(UInputComponent* PlayerInputComponent) override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void SetInteractionRayVisibility(EInteractionRayVisibility NewVisibility);

	UPROPERTY(EditAnywhere)
	UStaticMesh* InteractionRayMesh;

	UPROPERTY(BlueprintReadOnly)
	UStaticMeshComponent* InteractionRay;

	UPROPERTY(EditAnywhere)
	TEnumAsByte<EInteractionRayVisibility> InteractionRayVisibility = EInteractionRayVisibility::Invisible;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* IMCWidgetInteraction;

	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputAction* WidgetClickInputAction;

private:
	UFUNCTION()
	void OnBeginClick(const FInputActionValue& Value);

	UFUNCTION()
	void OnEndClick(const FInputActionValue& Value);

	void CreateInteractionRay();
	void SetupInteractionRay();
};
