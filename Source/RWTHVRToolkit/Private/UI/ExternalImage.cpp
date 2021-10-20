#include "UI/ExternalImage.h"

#include "Interfaces/IHttpResponse.h"
#include "HttpModule.h"
#include "Misc/FileHelper.h"
#include "Runtime/ImageWrapper/Public/IImageWrapperModule.h"

UExternalImage::UExternalImage(){}

void UExternalImage::LoadImageFromURL(const FString& ImageURL)
{
    const TFunction<void()> LoadingCallback = [this]()
    {
        if (LoadCompressedDataIntoTexture2D(CompressedData, NewTexture))
        {
            UImage::SetBrushFromTexture(NewTexture, true);
        }

        CompressedData.Empty();
    };

    if (FPaths::FileExists(ImageURL))
    {
        LoadDataFromFile(ImageURL, CompressedData, LoadingCallback);
    }
    else
    {
        LoadDataFromURL(ImageURL, CompressedData, LoadingCallback);
    }
}

bool UExternalImage::LoadCompressedDataIntoTexture2D(const TArray<uint8>& InCompressedData, UTexture2D*& OutTexture)
{
    TSharedPtr<IImageWrapper> ImageWrapper;
    TArray<uint8> UncompressedRGBA;

    /* Detect Format */
    IImageWrapperModule& Module = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	const EImageFormat DetectedFormat = Module.DetectImageFormat(InCompressedData.GetData(), InCompressedData.Num());
    if (DetectedFormat != EImageFormat::Invalid)
	{
		ImageWrapper = Module.CreateImageWrapper(DetectedFormat);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UExternalyLoadedImage with unknown format."));
		return false;
	}

    /* Decompress Data */
    if (!ImageWrapper.IsValid() || !ImageWrapper->SetCompressed(InCompressedData.GetData(), InCompressedData.Num()))
    {
        UE_LOG(LogTemp, Warning, TEXT("UExternalyLoadedImage is not able to decompress the image."));
        return false;
    }

    /* Write out data into decompressed array
     * Must be in BGRA channel format, otherwise the jpg format file cannot be loaded correctly */
    if (!ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedRGBA))
    {
        UE_LOG(LogTemp, Warning, TEXT("UExternalyLoadedImage is not able to write out the decompressed image."));
        return false;
    }

    OutTexture = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_B8G8R8A8);
    if (OutTexture)
    {
        void* TextureData = OutTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
        FMemory::Memcpy(TextureData, UncompressedRGBA.GetData(), UncompressedRGBA.Num());
        OutTexture->PlatformData->Mips[0].BulkData.Unlock();
        OutTexture->UpdateResource();
    }
    return true;
}

void UExternalImage::LoadDataFromURL(const FString& ImageURL, TArray<uint8>& OutCompressedData, TFunction<void()> OnSuccessCallback)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

	Request->OnProcessRequestComplete().BindLambda([OnSuccessCallback, &OutCompressedData](FHttpRequestPtr Request, FHttpResponsePtr Response, bool)
	{
		if (Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode()))
		{
            OutCompressedData.Empty();
            OutCompressedData.Append(Response->GetContent());
			OnSuccessCallback();
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("UExternalyLoadedImage Request unsucessful (%d): %s"), Response->GetResponseCode(), *Request->GetURL());
		}
	});

	Request->SetVerb("GET");
	Request->SetURL(ImageURL);
	Request->ProcessRequest();
}

void UExternalImage::LoadDataFromFile(const FString& ImagePath, TArray<uint8>& OutCompressedData, TFunction<void()> OnSuccessCallback)
{
    if(FFileHelper::LoadFileToArray(OutCompressedData, *ImagePath)) OnSuccessCallback();
}
