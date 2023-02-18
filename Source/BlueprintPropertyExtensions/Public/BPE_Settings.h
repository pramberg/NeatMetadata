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
	
	using FForEachCollectionSignature = void(UBPE_MetadataCollection&);
	void ForEachCollection(TFunctionRef<FForEachCollectionSignature> Functor) const;
	
protected:
	void RebuildMetadataCollections();
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostInitProperties() override;

	UPROPERTY(Config, EditDefaultsOnly, meta = (ShowDisplayNames))
	TArray<TSoftClassPtr<UBPE_MetadataCollection>> MetadataCollections;
	
	UPROPERTY()
	TArray<TObjectPtr<UBPE_MetadataCollection>> MetadataCollectionInstances;
};
