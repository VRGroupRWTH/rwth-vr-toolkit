#pragma once

#include "CoreMinimal.h"
#include "VRTransformRep.generated.h"

USTRUCT()
struct RWTHVRTOOLKIT_API FVRTransformRep
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(Transient)
	FVector Position;

	UPROPERTY(Transient)
	FRotator Rotation;

	FVRTransformRep()
	{
		Position = FVector::ZeroVector;
		Rotation = FRotator::ZeroRotator;
	}

	/**
	* @param Ar FArchive to read or write from.
	* @param Map PackageMap used to resolve references to UObject*
	* @param bOutSuccess return value to signify if the serialization was succesfull (if false, an error will be logged by the calling function)
	*
	* @return return true if the serialization was fully mapped. If false, the property will be considered 'dirty' and will replicate again on the next update.
	* This is needed for UActor* properties. If an actor's Actorchannel is not fully mapped, properties referencing it must stay dirty.
	* Note that UPackageMap::SerializeObject returns false if an object is unmapped. Generally, you will want to return false from your ::NetSerialize
	* if you make any calls to ::SerializeObject that return false.
	*
	*/
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		bOutSuccess = true;
		bOutSuccess &= SerializePackedVector<1, 24>(Position, Ar);
		Rotation.SerializeCompressed(Ar);
		return bOutSuccess;
	}
};