#pragma once

#include "Components/InputComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "CoreMinimal.h"
#include "DisplayClusterPawn.h"

#include "DisplayClusterPawnBase.generated.h"

UCLASS()
class NDISPLAYEXTENSIONS_API ADisplayClusterPawnBase : public ADisplayClusterPawn
{
  GENERATED_UCLASS_BODY()

public:
  UFUNCTION(BlueprintCallable, Category = "Pawn") virtual void MoveForward (float Value);
  UFUNCTION(BlueprintCallable, Category = "Pawn") virtual void MoveRight   (float Value);
  UFUNCTION(BlueprintCallable, Category = "Pawn") virtual void MoveUp      (float Value);
  UFUNCTION(BlueprintCallable, Category = "Pawn") virtual void TurnAtRate  (float Rate );
  UFUNCTION(BlueprintCallable, Category = "Pawn") virtual void TurnAtRate2 (float Rate );
  UFUNCTION(BlueprintCallable, Category = "Pawn") virtual void LookUpAtRate(float Rate );

  virtual void                    BeginPlay           ()                         override;
  virtual void                    Tick                (float DeltaSeconds)       override;
  virtual UPawnMovementComponent* GetMovementComponent()                   const override;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn") float BaseTurnRate  ;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn") float BaseLookUpRate;

protected:
  virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

  UPROPERTY(Category = Pawn, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true")) UFloatingPawnMovement*      MovementComponent ;
  UPROPERTY(Category = Pawn, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true")) URotatingMovementComponent* RotatingComponent ;
  UPROPERTY(Category = Pawn, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true")) URotatingMovementComponent* RotatingComponent2;
};