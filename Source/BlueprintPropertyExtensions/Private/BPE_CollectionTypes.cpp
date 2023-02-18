// Copyright Viktor Pramberg. All Rights Reserved.
#include "BPE_CollectionTypes.h"
#include "BPE_Module.h"

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
bool UBPE_MetadataCollection_Units::IsRelevantForContainedProperty(const FProperty& InProperty) const
{
	// It seems like enum properties created in BP are byte properties, not enum properties...
	if (const FByteProperty* AsByte = CastField<FByteProperty>(&InProperty))
	{
		return !AsByte->IsEnum();
	}
	
	return InProperty.IsA<FNumericProperty>() && !InProperty.IsA<FEnumProperty>();
}

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
bool UBPE_MetadataCollection_AssetBundles::IsRelevantForContainedProperty(const FProperty& InProperty) const
{
	const bool bIsApplicableProperty = InProperty.IsA<FSoftObjectProperty>() || InProperty.IsA<FSoftClassProperty>();
	return bIsApplicableProperty && InProperty.GetOwnerClass()->IsChildOf<UPrimaryDataAsset>();
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

#pragma region Color
UBPE_MetadataCollection_Color::UBPE_MetadataCollection_Color()
{
	Structs = {
		TBaseStructure<FLinearColor>::Get(),
		TBaseStructure<FColor>::Get(),
	};
}
#pragma endregion

#pragma region Title Property
bool UBPE_MetadataCollection_TitleProperty::IsRelevantForProperty(const FProperty& InProperty) const
{
	return InProperty.IsA<FArrayProperty>() && Super::IsRelevantForProperty(InProperty);
}

bool UBPE_MetadataCollection_TitleProperty::IsRelevantForContainedProperty(const FProperty& InProperty) const
{
	return InProperty.IsA<FStructProperty>();
}

#pragma endregion

#pragma region Get Options
TOptional<FString> UBPE_MetadataCollection_GetOptions::ValidateOptionsFunction(const FString& FunctionName) const
{
	const UFunction* Function;
	if (FunctionName.Contains(TEXT(".")))
	{
		// While this case works for static functions, they don't work with blueprint function libraries created in the editor, since they
		// implicitly get a world context object parameter in addition to any explicit values the user has added.

		Function = FindObject<UFunction>(nullptr, *FunctionName, true);

		if (Function)
		{
			if (!Function->HasAnyFunctionFlags(EFunctionFlags::FUNC_Static))
			{
				return FString(TEXT("A value with a \".\" is expected to be a static function."));
			}
			
			if (Function->GetOuterUClass()->IsAsset())
			{
				return FString::Printf(
					TEXT("%s is defined in script, inside of the %s function library. This is not supported, since functions defined like that always have one implicit parameter")
					TEXT(" which makes it incompatible with GetOption."), *Function->GetName(), *Function->GetOuterUClass()->GetPackage()->GetPathName()
				);
			}
		}
	}
	else
	{
		const FProperty* EditedProperty = CurrentWrapper.GetProperty();
		if (!ensure(EditedProperty))
		{
			return FString(TEXT("Internal Error!"));
		}
		
		Function = EditedProperty->GetOwnerClass()->FindFunctionByName(FName(FunctionName));
	}

	if (!Function)
	{
		return FString::Printf(TEXT("%s is not the name of a function."), *FunctionName);
	}

	if (Function->NumParms != 1)
	{
		return FString::Printf(TEXT("%s must have exactly one parameter."), *FunctionName);
	}
	
	const FProperty* ReturnValueProp = Function->GetReturnProperty();
	if (!ReturnValueProp)
	{
		return FString::Printf(TEXT("%s does not return a value. Ensure the return value is called \"ReturnValue\"."), *FunctionName);
	}

	const FArrayProperty* AsArray = CastField<FArrayProperty>(ReturnValueProp);
	if (!AsArray || !AsArray->Inner || !(AsArray->Inner->IsA<FStrProperty>() || AsArray->Inner->IsA<FNameProperty>()))
	{
		return FString::Printf(TEXT("%s does not return a valid value. The function must return an array of strings or names."), *FunctionName);
	}

	return {};
}

TOptional<FString> UBPE_MetadataCollection_GetOptions::GetValueForProperty(FProperty& Property) const
{
	if (Property.GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, GetOptions))
	{
		if (TOptional<FString> ErrorString = ValidateOptionsFunction(GetOptions))
		{
			// TODO: Figure out how to log this inside the Blueprint editor...
			UE_LOG(LogBlueprintPropertyExtensions, Error, TEXT("GetOptions[%s]: %s"), *CurrentWrapper.GetProperty()->GetName(), *ErrorString.GetValue());
			return NoPropertyValue;
		}
	}
	
	return Super::GetValueForProperty(Property);
}

bool UBPE_MetadataCollection_GetOptions::IsRelevantForContainedProperty(const FProperty& InProperty) const
{
	return InProperty.IsA<FStrProperty>() || InProperty.IsA<FNameProperty>();
}
#pragma endregion