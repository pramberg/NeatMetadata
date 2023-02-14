// Copyright Viktor Pramberg. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "BPE_Settings.generated.h"

class UBPE_MetadataCollection;

/**
 * 
 */
UCLASS(Config = "Editor", DefaultConfig)
class BLUEPRINTPROPERTYEXTENSIONS_API UBPE_Settings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UBPE_Settings();
	
	UPROPERTY(Config, EditDefaultsOnly)
	TArray<TSoftClassPtr<UBPE_MetadataCollection>> MetadataCollections;
};
