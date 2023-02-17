// Copyright Viktor Pramberg. All Rights Reserved.
#include "BPE_CollectionTypes.h"

#include "Curves/CurveLinearColor.h"
#include "Curves/CurveVector.h"

#pragma region Gameplay Tag Categories
UBPE_MetadataCollection_GameplayTagCategories::UBPE_MetadataCollection_GameplayTagCategories()
{
	Structs = {
		FGameplayTag::StaticStruct(),
		FGameplayTagContainer::StaticStruct(),
	};
}

TOptional<FString> UBPE_MetadataCollection_GameplayTagCategories::GetValueForProperty(FProperty& Property) const
{
	if (Property.GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, Categories))
	{
		// ToStringSimple adds a ", ". The space causes issues when parsing multiple tags...
		const FString Result = Categories.ToStringSimple().Replace(TEXT(" "), TEXT(""));
		return Result.IsEmpty() ? NoPropertyValue : Result;
	}
	
	return Super::GetValueForProperty(Property);
}

void UBPE_MetadataCollection_GameplayTagCategories::SetValueForProperty(const FProperty& Property, const FString& Value)
{
	if (Property.GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, Categories))
	{
		TArray<FString> Tags;
		Value.ParseIntoArray(Tags, TEXT(","));
		Categories.Reset(Tags.Num());
		for (const FString Tag : Tags)
		{
			Categories.AddTagFast(FGameplayTag::RequestGameplayTag(FName(Tag)));
		}
	}
	else
	{
		Super::SetValueForProperty(Property, Value);
	}
}
#pragma endregion 

#pragma region Units
TOptional<FString> UBPE_MetadataCollection_Units::GetValueForProperty(FProperty& Property) const
{
	if (Property.GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, Units) && Units == EUnit::Unspecified)
	{
		return NoPropertyValue;
	}
	
	if (Property.GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, ForceUnits) && ForceUnits == EUnit::Unspecified)
	{
		return NoPropertyValue;
	}
	
	return Super::GetValueForProperty(Property);
}

void UBPE_MetadataCollection_Units::SetValueForProperty(const FProperty& Property, const FString& Value)
{
	if (Property.GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, Units) && Value.IsEmpty())
	{
		Units = EUnit::Unspecified;
		return;
	}
	
	if (Property.GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, ForceUnits) && Value.IsEmpty())
	{
		ForceUnits = EUnit::Unspecified;
		return;
	}
	
	Super::SetValueForProperty(Property, Value);
}
#pragma endregion 

#pragma region Curves
UBPE_MetadataCollection_Curves::UBPE_MetadataCollection_Curves()
{
	Structs = {
		FRuntimeFloatCurve::StaticStruct(),
		FRuntimeVectorCurve::StaticStruct(),
		FRuntimeCurveLinearColor::StaticStruct(),
	};
}
#pragma endregion

#pragma region Asset Bundles
bool UBPE_MetadataCollection_AssetBundles::IsRelevantForProperty(FProperty* InProperty) const
{
	const bool bIsApplicableProperty = InProperty->IsA<FSoftObjectProperty>() || InProperty->IsA<FSoftClassProperty>();
	return bIsApplicableProperty && InProperty->GetOwnerClass()->IsChildOf<UPrimaryDataAsset>();
}

TOptional<FString> UBPE_MetadataCollection_AssetBundles::GetValueForProperty(FProperty& Property) const
{
	if (Property.GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, AssetBundles))
	{
		return FString::Join(AssetBundles, TEXT(","));
	}
	
	return Super::GetValueForProperty(Property);
}

void UBPE_MetadataCollection_AssetBundles::SetValueForProperty(const FProperty& Property, const FString& Value)
{
	if (Property.GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, AssetBundles))
	{
		AssetBundles.Empty();
		Value.ParseIntoArray(AssetBundles, TEXT(","));
	}
	else
	{
		Super::SetValueForProperty(Property, Value);
	}
}
#pragma endregion
