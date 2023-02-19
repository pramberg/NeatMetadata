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

	// Collections that should be created, and no other. If there are any entries in this array, *only* those
	// will be created. If this array is empty, all collections in the project will be created automatically.
	UPROPERTY(Config, EditDefaultsOnly, meta = (ShowDisplayNames))
	TArray<TSoftClassPtr<UBPE_MetadataCollection>> AllowedCollections;

	// Collections that aren't allowed to be automatically created.
	UPROPERTY(Config, EditDefaultsOnly, meta = (ShowDisplayNames))
	TArray<TSoftClassPtr<UBPE_MetadataCollection>> DisallowedCollections;
	
	UPROPERTY()
	TArray<TObjectPtr<UBPE_MetadataCollection>> MetadataCollectionInstances;
};
