// Copyright Viktor Pramberg. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "NeatMetadataSettings.generated.h"

class UNeatMetadataCollection;

/**
 * Project-wide settings for the Neat Metadata plugin. Configures what metadata that should be visible.
 */
UCLASS(MinimalAPI, Config = "Editor", DefaultConfig, DisplayName = "Neat Metadata")
class UNeatMetadataSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UNeatMetadataSettings();
	
	using FForEachCollectionSignature = void(UNeatMetadataCollection&);
	void ForEachCollection(TFunctionRef<FForEachCollectionSignature> Functor) const;

	// Tooltips for groups of metadata.
	UPROPERTY(Config, EditDefaultsOnly, Category = "Neat Metadata", AdvancedDisplay, meta = (MultiLine = "true"))
	TMap<FName, FText> GroupTooltips;
	
protected:
	void RebuildMetadataCollections();
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostInitProperties() override;

	// Collections that should be created, and no other. If there are any entries in this array, *only* those
	// will be created. If this array is empty, all collections in the project will be created automatically.
	UPROPERTY(Config, EditDefaultsOnly, Category = "Neat Metadata", meta = (ShowDisplayNames))
	TArray<TSoftClassPtr<UNeatMetadataCollection>> AllowedCollections;

	// Collections that aren't allowed to be automatically created.
	UPROPERTY(Config, EditDefaultsOnly, Category = "Neat Metadata", meta = (ShowDisplayNames))
	TArray<TSoftClassPtr<UNeatMetadataCollection>> DisallowedCollections;
	
	UPROPERTY()
	TArray<TObjectPtr<UNeatMetadataCollection>> MetadataCollectionInstances;
};

/**
 * User specific settings for the Neat Metadata plugin.
 */
UCLASS(MinimalAPI, Config = "EditorPerProjectUserSettings", DisplayName = "Neat Metadata")
class UNeatMetadataUserSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UNeatMetadataUserSettings();
	
	// Whether to show the "All Metadata" category in the property panel. Can be useful to debug all the metadata that has been
	// assigned to a property. For regular users this is not necessary and can make it easier to provide invalid values.
	UPROPERTY(Config, EditDefaultsOnly, DisplayName = "Show \"All Metadata\" Category", Category = "Neat Metadata")
	bool bShowAllMetadataCategory = false;
};