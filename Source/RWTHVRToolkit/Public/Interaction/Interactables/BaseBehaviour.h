// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "BaseBehaviour.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RWTHVRTOOLKIT_API UBaseBehaviour : public USceneComponent
{
	GENERATED_BODY()

public:
	/**
	 * Replication part
	 */

	/**
	 * Defines whether this behaviour will be executed on the server instead of the local client.
	 * If set to true, an RPC is sent to the server and it will not be run locally.
	 * */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bExecuteOnServer = false;

	/**
	 * Defines whether this behaviour will be executed on all connected clients that are relevant, including the
	 * local originator client. This only has an effect if bExecuteOnServer is true, as only the server can multicast
	 * to all other clients.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bExecuteOnServer"))
	bool bExecuteOnAllClients = false;
};
