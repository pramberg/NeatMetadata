// Copyright Viktor Pramberg. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BPE_MetadataCollection.h"
#include "BPE_CollectionTypes.generated.h"


/**
 * Exposes the `Categories` metadata for gameplay tag properties.
 * @see FGameplayTag, FGameplayTagContainer
 */
UCLASS(meta=(DisplayName = "Gameplay Tag Categories"))
class UBPE_MetadataCollection_GameplayTagCategories : public UBPE_MetadataCollectionStruct
{
	GENERATED_BODY()

public:
	UBPE_MetadataCollection_GameplayTagCategories();

	UPROPERTY(EditAnywhere)
	FGameplayTagContainer Categories;
	
protected:
	virtual FString GetValueForProperty(FProperty& Property) const override;
	virtual void SetValueForProperty(const FProperty& Property, const FString& Value) override;
};

/**
 * Exposes edit condition and related metadata. This may be used to disable editing properties
 * or even hiding properties that can't be edited.
 */
UCLASS(meta=(DisplayName = "Edit Condition"))
class UBPE_MetadataCollection_EditCondition : public UBPE_MetadataCollection
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FString EditCondition;

	UPROPERTY(EditAnywhere)
	bool EditConditionHides;

	UPROPERTY(EditAnywhere)
	bool InlineEditConditionToggle;
};

/**
 * Exposes the Units/ForceUnits metadata to numeric properties. This can be used to specify what unit the property holds.
 */
UCLASS(meta=(DisplayName = "Units"))
class UBPE_MetadataCollection_Units : public UBPE_MetadataCollection
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	EUnit Units;

	UPROPERTY(EditAnywhere)
	EUnit ForceUnits;
};

/**
 * Exposes XAxisName and YAxisName on curve properties.
 */
UCLASS(meta=(DisplayName = "Curves"))
class UBPE_MetadataCollection_Curves : public UBPE_MetadataCollectionStruct
{
	GENERATED_BODY()

public:
	UBPE_MetadataCollection_Curves();

	// The name of the X-axis of the curve.
	UPROPERTY(EditAnywhere)
	FString XAxisName;

	// The name of the Y-axis of the curve.
	UPROPERTY(EditAnywhere)
	FString YAxisName;
};

/**
 * Exposes the possibility to add Asset Bundles to Primary Assets.
 * @see UAssetManager::InitializeAssetBundlesFromMetadata
 */
UCLASS(meta=(DisplayName = "Asset Bundles"))
class UBPE_MetadataCollection_AssetBundles : public UBPE_MetadataCollectionStruct
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TArray<FString> AssetBundles;

protected:
	virtual bool IsRelevantForProperty(FProperty* InProperty) const override;
	virtual FString GetValueForProperty(FProperty& Property) const override;
	virtual void SetValueForProperty(const FProperty& Property, const FString& Value) override;
};