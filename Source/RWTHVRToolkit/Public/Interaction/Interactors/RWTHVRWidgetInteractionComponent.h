// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetInteractionComponent.h"
#include "Pawn/InputExtensionInterface.h"
#include "RWTHVRWidgetInteractionComponent.generated.h"

UENUM()
enum EInteractionRayVisibility
{
	Visible UMETA(DisplayName = "Interaction ray visible"),
	VisibleOnHoverOnly UMETA(DisplayName =
								 "Interaction ray only visible when hovering over interactable world UI widgets"),
	Invisible UMETA(DisplayName = "Interaction ray invisible")
};


UCLASS(Blueprintable, Abstract, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RWTHVRTOOLKIT_API URWTHVRWidgetInteractionComponent : public UWidgetInteractionComponent,
															public IInputExtensionInterface
{
	GENERATED_BODY()

public:
	URWTHVRWidgetInteractionComponent();

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

	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputAction* WidgetLeftClickInputAction;
	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputAction* WidgetRightClickInputAction;

private:
	UFUNCTION()
	void OnBeginLeftClick(const FInputActionValue& Value);

	UFUNCTION()
	void OnEndLeftClick(const FInputActionValue& Value);

	UFUNCTION()
	void OnBeginRightClick(const FInputActionValue& Value);

	UFUNCTION()
	void OnEndRightClick(const FInputActionValue& Value);

	void CreateInteractionRay();
	void SetupInteractionRay();
};
