// Copyright Viktor Pramberg. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "BPE_Settings.generated.h"

class UBPE_MetadataCollection;

/**
 * Project-wide settings for the Blueprint Property Extensions plugin. Configures what metadata that should be visible.
 */
UCLASS(MinimalAPI, Config = "Editor", DefaultConfig, DisplayName = "Blueprint Property Extensions")
class UBPE_Settings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UBPE_Settings();
	
	using FForEachCollectionSignature = void(UBPE_MetadataCollection&);
	void ForEachCollection(TFunctionRef<FForEachCollectionSignature> Functor) const;

	// Tooltips for groups of metadata.
	UPROPERTY(Config, EditDefaultsOnly, Category = "Blueprint Property Extensions", AdvancedDisplay, meta = (MultiLine = "true"))
	TMap<FName, FText> GroupTooltips;
	
protected:
	void RebuildMetadataCollections();
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostInitProperties() override;

	// Collections that should be created, and no other. If there are any entries in this array, *only* those
	// will be created. If this array is empty, all collections in the project will be created automatically.
	UPROPERTY(Config, EditDefaultsOnly, Category = "Blueprint Property Extensions", meta = (ShowDisplayNames))
	TArray<TSoftClassPtr<UBPE_MetadataCollection>> AllowedCollections;

	// Collections that aren't allowed to be automatically created.
	UPROPERTY(Config, EditDefaultsOnly, Category = "Blueprint Property Extensions", meta = (ShowDisplayNames))
	TArray<TSoftClassPtr<UBPE_MetadataCollection>> DisallowedCollections;
	
	UPROPERTY()
	TArray<TObjectPtr<UBPE_MetadataCollection>> MetadataCollectionInstances;
};

/**
 * User specific settings for the Blueprint Property Extensions plugin.
 */
UCLASS(MinimalAPI, Config = "EditorPerProjectUserSettings", DisplayName = "Blueprint Property Extensions")
class UBPE_UserSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UBPE_UserSettings();
	
	// Whether to show the "All Metadata" category in the property panel. Can be useful to debug all the metadata that has been
	// assigned to a property. For regular users this is not necessary and can make it easier to provide invalid values.
	UPROPERTY(Config, EditDefaultsOnly, DisplayName = "Show \"All Metadata\" Category", Category = "Blueprint Property Extensions")
	bool bShowAllMetadataCategory = false;
};