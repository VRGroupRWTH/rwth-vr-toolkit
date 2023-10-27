#pragma once

#include "CoreMinimal.h"
#include "Components/Image.h"
#include "ExternalImage.generated.h"

/**
 * This class is a simple wrapper around the UImage class that can load images from URLs and local file paths
 */
UCLASS()
class RWTHVRTOOLKIT_API UExternalImage : public UImage
{
	GENERATED_BODY()

	UExternalImage();

public:
	/* Loads an Image from either a file or an URL */
	UFUNCTION(BlueprintCallable)
	void LoadImageFromURL(const FString& ImageURL);

private:
	TArray<uint8> CompressedData;
	UPROPERTY()
	UTexture2D* NewTexture;

	bool LoadCompressedDataIntoTexture2D(const TArray<uint8>& InCompressedData, UTexture2D*& OutTexture);
	static void LoadDataFromURL(const FString& ImageURL, TArray<uint8>& OutCompressedData,
	                            TFunction<void()> OnSuccessCallback);
	static void LoadDataFromFile(const FString& ImagePath, TArray<uint8>& OutCompressedData,
	                             TFunction<void()> OnSuccessCallback);
};
