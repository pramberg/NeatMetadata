// Copyright Viktor Pramberg. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BPE_MetadataCollection.h"
#include "GameplayTagContainer.h"
#include "Math/UnitConversion.h"
#include "BPE_CollectionTypes.generated.h"


/**
 * Controls what "Categories", or root gameplay tags can be selected on a GameplayTag
 * or GameplayTagContainer property. Use this if you only want specific tags to be selectable.
 *
 * @see FGameplayTag, FGameplayTagContainer, UGameplayTagsManager::StaticGetCategoriesMetaFromPropertyHandle
 */
UCLASS(meta=(DisplayName = "Gameplay Tag Categories"))
class UBPE_MetadataCollection_GameplayTagCategories : public UBPE_MetadataCollectionStruct
{
	GENERATED_BODY()

public:
	UBPE_MetadataCollection_GameplayTagCategories();

	// One or more root tags that are selectable in the GameplayTag widget.
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer Categories;
	
protected:
	virtual TOptional<FString> GetValueForProperty(FProperty& Property) const override;
	virtual void SetValueForProperty(const FProperty& Property, const FString& Value) override;
};

/**
 * Controls what conditions need to be met in order to edit the current property. Also allows for hiding
 * properties that cannot be edited.
 */
UCLASS(meta=(DisplayName = "Edit Condition"))
class UBPE_MetadataCollection_EditCondition : public UBPE_MetadataCollection
{
	GENERATED_BODY()

public:
	// Condition used to enable editing this property. A basic expression would be `bMyBoolVariable`.
	// That would disable editing of the property if `bMyBoolVariable` is false.
	//
	// The syntax is fairly expressive, so you can also do things like:
	// `MyFloatVariable > 0.0`, `!bMyBoolVariable`, `MyAssetEnum == 2` or `MyCppEnum == EnumType::Value`.
	//
	// NOTE: Be aware of the difference between enums defined as an asset in the editor vs. enums defined in C++.
	// Enums defined in C++ can be referred to as `EnumType::Value`, while that's not possible for asset enums.
	// The name of asset enum names are all called `EnumType::NewEnumeratorX`, where X is an incremented index.
	// It is therefore best to refer to asset enum values by index instead.
	UPROPERTY(EditAnywhere)
	FString EditCondition;

	// If the edit condition isn't met, should the property be hidden?
	UPROPERTY(EditAnywhere)
	bool EditConditionHides;

	// If this boolean property is used as an edit condition, should it be inlined next to the property?
	UPROPERTY(EditAnywhere)
	bool InlineEditConditionToggle;
};

/**
 * Controls what unit this property represents. For example, a float variable can represent
 * distance, angles or speed. By specifying a unit, you can make it easier for a user to
 * understand the use of the property.
 */
UCLASS(meta=(DisplayName = "Units"))
class UBPE_MetadataCollection_Units : public UBPE_MetadataCollection
{
	GENERATED_BODY()

public:
	// Specifies what unit this property represents. The value of the property is automatically converted
	// from the user's preferred unit for that value type. If you specify `Centimeter` and the user's preferred
	// unit is `Yards`, Unreal will handle the conversion from yards to centimeters automatically.
	UPROPERTY(EditAnywhere)
	EUnit Units = EUnit::Unspecified;

	// Forces the unit of this property to be the selected type, regardless of user preferences.
	// For example, if you specify `Centimeters` and the user uses `Yards`, the property will still
	// use `Centimeters`.
	//
	// Note that this property is prioritized over `Units`. If you specify both `Units` and `ForceUnits`,
	// the behavior of `ForceUnits` will be applied.
	UPROPERTY(EditAnywhere)
	EUnit ForceUnits = EUnit::Unspecified;

protected:
	virtual TOptional<FString> GetValueForProperty(FProperty& Property) const override;
	virtual void SetValueForProperty(const FProperty& Property, const FString& Value) override;
};

/**
 * Allows you to specify the name of the axes on runtime curve properties.
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
 * Controls what Asset Bundles this property belongs to. Doing that will 
 *
 * @see https://docs.unrealengine.com/5.1/en-US/asset-management-in-unreal-engine/#assetbundles
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
	virtual TOptional<FString> GetValueForProperty(FProperty& Property) const override;
	virtual void SetValueForProperty(const FProperty& Property, const FString& Value) override;
};