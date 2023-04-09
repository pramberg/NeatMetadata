// Copyright Viktor Pramberg. All Rights Reserved.
#include "NeatMetadataCollections.h"
#include "NeatMetadataModule.h"

#include "Widgets/SNeatInterfaceSelector.h"
#include "Widgets/SNeatFunctionSelector.h"

#include "Curves/CurveLinearColor.h"
#include "Curves/CurveVector.h"

#include "Kismet2/BlueprintEditorUtils.h"
#include "BlueprintEditorModule.h"
#include "DataTableEditorUtils.h"

#include "Algo/Transform.h"

namespace
{
	bool IsNumericProperty(const FProperty& InProperty)
	{
		// It seems like enum properties created in BP are byte properties, not enum properties...
		if (const FByteProperty* AsByte = CastField<FByteProperty>(&InProperty))
		{
			return !AsByte->IsEnum();
		}
	
		return InProperty.IsA<FNumericProperty>() && !InProperty.IsA<FEnumProperty>();
	}
}

#pragma region Edit Condition
bool UNeatMetadataCollection_EditCondition::IsPropertyVisible(const FProperty& Property) const
{
	if (!Super::IsPropertyVisible(Property))
	{
		return false;
	}

	static const FName InlineEditConditionToggleName(GET_MEMBER_NAME_CHECKED(ThisClass, InlineEditConditionToggle));
	if (Property.GetFName() == InlineEditConditionToggleName)
	{
		return CurrentWrapper.GetProperty()->IsA<FBoolProperty>();
	}

	static const FName EditConditionHidesName(GET_MEMBER_NAME_CHECKED(ThisClass, EditConditionHides));
	if (Property.GetFName() == EditConditionHidesName)
	{
		return !EditCondition.IsEmpty();
	}
	
	static const FName HideEditConditionToggleName(GET_MEMBER_NAME_CHECKED(ThisClass, HideEditConditionToggle));
	if (Property.GetFName() == HideEditConditionToggleName)
	{
		if (EditCondition.IsEmpty())
		{
			return false;
		}
		
		const FProperty* EditConditionProperty = CurrentWrapper.GetProperty()->GetOwnerClass()->FindPropertyByName(FName(EditCondition));
		if (!EditConditionProperty)
		{
			// If we don't find a property we assume it's a more complicated expression. Those are not supported as inline edit conditions, so hide this property.
			return false;
		}
		
		return EditConditionProperty->IsA<FBoolProperty>() && EditConditionProperty->HasMetaData(InlineEditConditionToggleName);
	}
	
	return true;
}
#pragma endregion

#pragma region Gameplay Tag Categories
UNeatMetadataCollection_GameplayTagCategories::UNeatMetadataCollection_GameplayTagCategories()
{
	Structs = {
		FGameplayTag::StaticStruct(),
		FGameplayTagContainer::StaticStruct(),
	};
}

TOptional<FString> UNeatMetadataCollection_GameplayTagCategories::ExportValueForProperty(FProperty& Property) const
{
	if (Property.GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, Categories))
	{
		// ToStringSimple adds a ", ". The space causes issues when parsing multiple tags...
		const FString Result = Categories.ToStringSimple().Replace(TEXT(" "), TEXT(""));
		return Result.IsEmpty() ? NullOpt : TOptional(Result);
	}
	
	return Super::ExportValueForProperty(Property);
}

void UNeatMetadataCollection_GameplayTagCategories::ImportValueForProperty(const FProperty& Property, const FString& Value)
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
		Super::ImportValueForProperty(Property, Value);
	}
}
#pragma endregion 

#pragma region Units
bool UNeatMetadataCollection_Units::IsRelevantForContainedProperty(const FProperty& InProperty) const
{
	return IsNumericProperty(InProperty);
}

TOptional<FString> UNeatMetadataCollection_Units::ExportValueForProperty(FProperty& Property) const
{
	if (Property.GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, Units) && Units == EUnit::Unspecified)
	{
		return {};
	}
	
	if (Property.GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, ForceUnits) && ForceUnits == EUnit::Unspecified)
	{
		return {};
	}
	
	return Super::ExportValueForProperty(Property);
}

void UNeatMetadataCollection_Units::ImportValueForProperty(const FProperty& Property, const FString& Value)
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
	
	Super::ImportValueForProperty(Property, Value);
}
#pragma endregion 

#pragma region Curves
UNeatMetadataCollection_Curves::UNeatMetadataCollection_Curves()
{
	Structs = {
		FRuntimeFloatCurve::StaticStruct(),
		FRuntimeVectorCurve::StaticStruct(),
		FRuntimeCurveLinearColor::StaticStruct(),
	};
}
#pragma endregion

#pragma region Asset Bundles
bool UNeatMetadataCollection_AssetBundles::IsRelevantForContainedProperty(const FProperty& InProperty) const
{
	const bool bIsApplicableProperty = InProperty.IsA<FSoftObjectProperty>() || InProperty.IsA<FSoftClassProperty>();
	return bIsApplicableProperty && InProperty.GetOwnerClass()->IsChildOf<UPrimaryDataAsset>();
}

TOptional<FString> UNeatMetadataCollection_AssetBundles::ExportValueForProperty(FProperty& Property) const
{
	if (Property.GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, AssetBundles))
	{
		const FString Result = FString::Join(AssetBundles, TEXT(","));
		return AssetBundles.IsEmpty() ? NullOpt : TOptional(Result);
	}
	
	return Super::ExportValueForProperty(Property);
}

void UNeatMetadataCollection_AssetBundles::ImportValueForProperty(const FProperty& Property, const FString& Value)
{
	if (Property.GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, AssetBundles))
	{
		if (!Value.IsEmpty())
		{
			AssetBundles.Empty();
			Value.ParseIntoArray(AssetBundles, TEXT(","), false);
		}
	}
	else
	{
		Super::ImportValueForProperty(Property, Value);
	}
}
#pragma endregion

#pragma region Color
UNeatMetadataCollection_Color::UNeatMetadataCollection_Color()
{
	Structs = {
		TBaseStructure<FLinearColor>::Get(),
		TBaseStructure<FColor>::Get(),
	};
}
#pragma endregion

#pragma region Title Property
bool UNeatMetadataCollection_TitleProperty::IsRelevantForProperty(const FProperty& InProperty) const
{
	return InProperty.IsA<FArrayProperty>() && Super::IsRelevantForProperty(InProperty);
}

bool UNeatMetadataCollection_TitleProperty::IsRelevantForContainedProperty(const FProperty& InProperty) const
{
	return InProperty.IsA<FStructProperty>();
}
#pragma endregion

#pragma region Get Options
namespace
{
	bool GetOptionsFunctionFilter(const UFunction* InFunction, bool bInIsMemberFunction)
	{
		if (!InFunction->HasAnyFunctionFlags(FUNC_Static) && !bInIsMemberFunction)
		{
			return false;
		}
		
		const FArrayProperty* AsArray = CastField<FArrayProperty>(InFunction->GetReturnProperty());
		if (!AsArray || !AsArray->Inner || !(AsArray->Inner->IsA<FStrProperty>() || AsArray->Inner->IsA<FNameProperty>()))
		{
			return false;
		}
		
		if (InFunction->NumParms != 1)
		{
			return false;
		}
		return true;
	}
}

TSharedPtr<SWidget> UNeatMetadataCollection_GetOptions::CreateValueWidgetForProperty(const TSharedRef<IPropertyHandle>& InHandle)
{
	if (InHandle->GetProperty()->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, GetOptions))
	{
		return SNew(SNeatFunctionSelector, InHandle)
		.FunctionFilter_Static(&GetOptionsFunctionFilter)
		.AddNewFunction_UObject(this, &ThisClass::OnAddNewFunction)
		.MemberClass(CurrentWrapper.GetProperty()->GetOwnerClass());
	}
	
	return Super::CreateValueWidgetForProperty(InHandle);
}

TOptional<FString> UNeatMetadataCollection_GetOptions::ValidateOptionsFunction(const FString& FunctionName) const
{
	if (FunctionName.IsEmpty())
	{
		return NullOpt;
	}
	
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

TOptional<FString> UNeatMetadataCollection_GetOptions::ExportValueForProperty(FProperty& Property) const
{
	if (Property.GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, GetOptions))
	{
		if (TOptional<FString> ErrorString = ValidateOptionsFunction(GetOptions))
		{
			// TODO: Figure out how to log this inside the Blueprint editor...
			UE_LOG(LogNeatMetadata, Error, TEXT("GetOptions[%s]: %s"), *CurrentWrapper.GetProperty()->GetName(), *ErrorString.GetValue());
			return {};
		}
	}
	
	return Super::ExportValueForProperty(Property);
}

bool UNeatMetadataCollection_GetOptions::IsRelevantForContainedProperty(const FProperty& InProperty) const
{
	bIsString = InProperty.IsA<FStrProperty>();
	return bIsString || InProperty.IsA<FNameProperty>();
}

TOptional<FString> UNeatMetadataCollection_GetOptions::OnAddNewFunction() const
{
	UBlueprint* BP = CurrentWrapper.GetBlueprint();
	
	const FScopedTransaction Transaction(INVTEXT("Add New Function")); 
	BP->Modify();

	const FString DesiredName = FString::Printf(TEXT("%s_Options"), *CurrentWrapper.GetProperty()->GetName());
	const FName UniqueName = FBlueprintEditorUtils::FindUniqueKismetName(BP, DesiredName);
	
	UEdGraph* NewGraph = FBlueprintEditorUtils::CreateNewGraph(BP, UniqueName, UEdGraph::StaticClass(), UEdGraphSchema_K2::StaticClass());

	static const UFunction* StringFunc = StaticClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(ThisClass, GetOptionsStringSignature));
	static const UFunction* NameFunc = StaticClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(ThisClass, GetOptionsNameSignature));
	FBlueprintEditorUtils::AddFunctionGraph(BP, NewGraph, true, bIsString ? StringFunc : NameFunc);

	{
		UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
		IBlueprintEditor* Editor = static_cast<IBlueprintEditor*>(AssetEditorSubsystem->FindEditorForAsset(BP, false));
		Editor->JumpToHyperlink(NewGraph, true);
	}

	return UniqueName.ToString();
}
#pragma endregion

#pragma region Directory Path
UNeatMetadataCollection_DirectoryPath::UNeatMetadataCollection_DirectoryPath()
{
	// FDirectoryPath doesn't have a StaticStruct() member nor a TBaseStructure<FDirectoryPath>::Get() implementation.
	// Therefore have to find it manually.
	static UScriptStruct* DirectoryPathStruct = FindObjectChecked<UScriptStruct>(UObject::StaticClass()->GetPackage(), TEXT("DirectoryPath"));
	Structs = {
		DirectoryPathStruct
	};
}
#pragma endregion 

#pragma region File Path
UNeatMetadataCollection_FilePath::UNeatMetadataCollection_FilePath()
{
	// FFilePath doesn't have a StaticStruct() member nor a TBaseStructure<FFilePath>::Get() implementation.
	// Therefore have to find it manually.
	static UScriptStruct* FilePathStruct = FindObjectChecked<UScriptStruct>(UObject::StaticClass()->GetPackage(), TEXT("FilePath"));
	Structs = {
		FilePathStruct
	};
}

TOptional<FString> UNeatMetadataCollection_FilePath::ExportValueForProperty(FProperty& Property) const
{
	if (Property.GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, FilePathFilter_Internal))
	{
		SetFilePathFilterMetadata();
	}
	
	return Super::ExportValueForProperty(Property);
}

void UNeatMetadataCollection_FilePath::SetFilePathFilterMetadata() const
{
	// The setup for FilePathFilter is a bit more complicated than typical.
	// We could either parse the metadata string like what happens in trivial cases where we just need a slightly different
	// format on the string. But due to the potentially complex format of the filter string, I prefer to store the raw
	// filter "description" in a separate field, and generate the real filter based on that description. This solution vastly
	// reduces the complexity, but of course introduces a new metadata field. I think that's a fair compromise, since
	// property metadata is just an editor feature anyways.
	static const FName FilePathFilter("FilePathFilter");
	
	TArray<FNeatFilePathFilter> FilePathDescCopy = FilePathFilter_Internal;
	FilePathDescCopy.RemoveAll([](const FNeatFilePathFilter& Filter) { return Filter.Extension.IsEmpty(); });
	
	if (FilePathDescCopy.IsEmpty())
	{
		CurrentWrapper.RemoveMetadata(FilePathFilter);
		return;
	}
		
	const FString IndividualFiles = FString::JoinBy(FilePathDescCopy, TEXT("|"), [](const FNeatFilePathFilter& Filter)
	{
		const FString Description = Filter.Description.IsEmpty() ? FString::Printf(TEXT("%s Files"), *Filter.Extension.ToUpper()) : Filter.Description;
		return FString::Printf(TEXT("%s (*.%s)|*.%s"), *Description, *Filter.Extension, *Filter.Extension);
	});

	if (FilePathDescCopy.Num() == 1)
	{
		CurrentWrapper.SetMetadata(FilePathFilter, IndividualFiles);
		return;
	}
		
	const FString CombinedFiles = FString::JoinBy(FilePathDescCopy, TEXT(";"), [](const FNeatFilePathFilter& Filter)
	{
		return FString::Printf(TEXT("*.%s"), *Filter.Extension);
	});

	CurrentWrapper.SetMetadata(FilePathFilter, FString::Printf(TEXT("All Files (%s)|%s|%s"), *CombinedFiles, *CombinedFiles, *IndividualFiles));
}
#pragma endregion

#pragma region Primary Asset Id
UNeatMetadataCollection_PrimaryAssetId::UNeatMetadataCollection_PrimaryAssetId()
{
	Structs = {
		TBaseStructure<FPrimaryAssetId>::Get(),
	};
}

TOptional<FString> UNeatMetadataCollection_PrimaryAssetId::ExportValueForProperty(FProperty& Property) const
{
	if (Property.GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, AllowedTypes))
	{
		const FString Result = FString::JoinBy(AllowedTypes, TEXT(","), [](const FPrimaryAssetType& Type) { return Type.ToString(); });
		return AllowedTypes.IsEmpty() ? NullOpt : TOptional(Result);
	}
	
	return Super::ExportValueForProperty(Property);
}

void UNeatMetadataCollection_PrimaryAssetId::ImportValueForProperty(const FProperty& Property, const FString& Value)
{
	if (Property.GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, AllowedTypes) && !Value.IsEmpty())
	{
		TArray<FString> TypesAsString;
		Value.ParseIntoArray(TypesAsString, TEXT(","));

		AllowedTypes.Empty(TypesAsString.Num());
		Algo::Transform(TypesAsString, AllowedTypes, [](const FString& TypeAsString){ return FPrimaryAssetType(*TypeAsString); });
	}
	else
	{
		Super::ImportValueForProperty(Property, Value);
	}
}
#pragma endregion

#pragma region Show Only Inner Properties
bool UNeatMetadataCollection_ShowOnlyInnerProperties::IsRelevantForContainedProperty(const FProperty& InProperty) const
{
	return InProperty.IsA<FStructProperty>();
}
#pragma endregion

#pragma region Class Picker
bool UNeatMetadataCollection_ClassPicker::IsRelevantForContainedProperty(const FProperty& InProperty) const
{
	return InProperty.IsA<FSoftClassProperty>() || InProperty.IsA<FClassProperty>();
}

TSharedPtr<SWidget> UNeatMetadataCollection_ClassPicker::CreateValueWidgetForProperty(const TSharedRef<IPropertyHandle>& InHandle)
{
	if (InHandle->GetProperty()->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, MustImplement))
	{
		return SNew(SNeatInterfaceSelector, InHandle);
	}
	
	return Super::CreateValueWidgetForProperty(InHandle);
}
#pragma endregion

#pragma region Array
bool UNeatMetadataCollection_Array::IsRelevantForProperty(const FProperty& InProperty) const
{
	return InProperty.IsA<FArrayProperty>();
}
#pragma endregion

#pragma region Numbers
TArray<FString> UNeatMetadataCollection_Numbers::GetAllArrayProperties() const
{
	TArray<FString> Results;
	Results.Add(TEXT("None"));
	for (const FArrayProperty* Property : TFieldRange<FArrayProperty>(CurrentWrapper.GetProperty()->GetOwnerClass()))
	{
		if (!Property->HasAnyPropertyFlags(CPF_DisableEditOnInstance))
		{
			Results.Add(Property->GetName());
		}
	}
	return Results;
}

TOptional<FString> UNeatMetadataCollection_Numbers::ExportValueForProperty(FProperty& Property) const
{
	if (Property.GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, ArrayClamp))
	{
		return (ArrayClamp.IsEmpty() || ArrayClamp == TEXT("None")) ? NullOpt : TOptional(ArrayClamp);
	}
	
	return Super::ExportValueForProperty(Property);
}

bool UNeatMetadataCollection_Numbers::IsRelevantForContainedProperty(const FProperty& InProperty) const
{
	return IsNumericProperty(InProperty);
}

bool UNeatMetadataCollection_Numbers::IsPropertyVisible(const FProperty& Property) const
{
	if (!Super::IsPropertyVisible(Property))
	{
		return false;
	}

	static const FName ArrayClampName(GET_MEMBER_NAME_CHECKED(ThisClass, ArrayClamp));
	static const FName MultipleName(GET_MEMBER_NAME_CHECKED(ThisClass, Multiple));
	if (Property.GetFName() == ArrayClampName || Property.GetFName() == MultipleName)
	{
		const FNumericProperty* AsNumeric = CastField<FNumericProperty>(CurrentWrapper.GetProperty());
		return AsNumeric && AsNumeric->IsInteger();
	}
	
	return true;
}
#pragma endregion

#pragma region AllowPreserveRatio
bool UNeatMetadataCollection_AllowPreserveRatio::IsRelevantForContainedProperty(const FProperty& InProperty) const
{
	if (const FStructProperty* AsStructProperty = CastField<FStructProperty>(&InProperty))
	{
		const UStruct* Struct = AsStructProperty->Struct;
		return
		(
			Struct == TBaseStructure<FVector>::Get() ||
			Struct == TBaseStructure<FVector2D>::Get() ||
			Struct == TBaseStructure<FVector4>::Get() ||
			Struct == TBaseStructure<FRotator>::Get()
			// Int vectors are broken in 5.1.1. It applies properly when increasing values, but not when decreasing.
			//Struct == TBaseStructure<FIntPoint>::Get() ||
			//Struct == TBaseStructure<FIntVector>::Get() ||
			//Struct == TBaseStructure<FIntVector4>::Get()
		);
	}
	return false;
}
#pragma endregion

#pragma region Assets
bool UNeatMetadataCollection_Assets::IsRelevantForContainedProperty(const FProperty& InProperty) const
{
	return (InProperty.IsA<FObjectPropertyBase>() || InProperty.IsA<FInterfaceProperty>()) && !(InProperty.IsA<FClassProperty>() || InProperty.IsA<FSoftClassProperty>());
}

TOptional<FString> UNeatMetadataCollection_Assets::ExportValueForProperty(FProperty& Property) const
{
	const auto SetActualMetadata = [this](FName InMetadataName, TArray<FNeatAssetDataTagKeyValue> InArray)
	{
		InArray.RemoveAll([](const FNeatAssetDataTagKeyValue& InTag) { return InTag.Key.IsEmpty(); });
		
		if (InArray.IsEmpty())
		{
			CurrentWrapper.RemoveMetadata(InMetadataName);
		}
		else
		{
			const FString Result = FString::JoinBy(InArray, TEXT(","), [](const FNeatAssetDataTagKeyValue& InTag)
			{
				return InTag.Value.IsEmpty() ? InTag.Key : FString::Printf(TEXT("%s=%s"), *InTag.Key, *InTag.Value);
			});
		
			CurrentWrapper.SetMetadata(InMetadataName, Result);
		}
	};
	
	if (Property.GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, RequiredAssetDataTags_Internal))
	{
		static const FName RequiredAssetDataTags("RequiredAssetDataTags");
		SetActualMetadata(RequiredAssetDataTags, RequiredAssetDataTags_Internal);
	}
	else if (Property.GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, DisallowedAssetDataTags_Internal))
	{
		static const FName DisallowedAssetDataTags("DisallowedAssetDataTags");
		SetActualMetadata(DisallowedAssetDataTags, DisallowedAssetDataTags_Internal);
	}
	else if (Property.GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, AllowedClasses))
	{
		const FString Result = FString::JoinBy(AllowedClasses, TEXT(","), [](const TSoftClassPtr<UObject>& Type) { return Type ? Type.ToString() : TEXT("None"); });
		return AllowedClasses.IsEmpty() ? NullOpt : TOptional(Result);
	}
	else if (Property.GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, DisallowedClasses))
	{
		const FString Result = FString::JoinBy(DisallowedClasses, TEXT(","), [](const TSoftClassPtr<UObject>& Type) { return Type ? Type.ToString() : TEXT("None"); });
		return DisallowedClasses.IsEmpty() ? NullOpt : TOptional(Result);
	}

	return Super::ExportValueForProperty(Property);
}

void UNeatMetadataCollection_Assets::ImportValueForProperty(const FProperty& Property, const FString& Value)
{
	if (Property.GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, AllowedClasses) && !Value.IsEmpty())
	{
		TArray<FString> TypesAsString;
		Value.ParseIntoArray(TypesAsString, TEXT(","));

		AllowedClasses.Empty(TypesAsString.Num());
		Algo::Transform(TypesAsString, AllowedClasses, [](const FString& TypeAsString){ return TSoftClassPtr(FSoftClassPath(TypeAsString)); });
	}
	else if (Property.GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, DisallowedClasses) && !Value.IsEmpty())
	{
		TArray<FString> TypesAsString;
		Value.ParseIntoArray(TypesAsString, TEXT(","));

		DisallowedClasses.Empty(TypesAsString.Num());
		Algo::Transform(TypesAsString, DisallowedClasses, [](const FString& TypeAsString){ return TSoftClassPtr(FSoftClassPath(TypeAsString)); });
	}
	else
	{
		Super::ImportValueForProperty(Property, Value);
	}
}
#pragma endregion

#pragma region Row Type
UNeatMetadataCollection_RowType::UNeatMetadataCollection_RowType()
{
	Structs.Add(TBaseStructure<FDataTableRowHandle>::Get());
}

TArray<FString> UNeatMetadataCollection_RowType::GetPossibleRowTypes()
{
	TArray<FAssetData> Assets;
	FDataTableEditorUtils::GetPossibleStructAssetData(Assets);
	
	TArray<FString> Rows;
	Rows.Add("None");
	for (const FAssetData& Asset : Assets)
	{
		Rows.Add(Asset.GetSoftObjectPath().GetAssetPathString());
	}

	return Rows;
}

TOptional<FString> UNeatMetadataCollection_RowType::ExportValueForProperty(FProperty& Property) const
{
	if (Property.GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, RowType))
	{
		return (RowType.IsEmpty() || RowType == TEXT("None")) ? NullOpt : TOptional(RowType);
	}
	
	return Super::ExportValueForProperty(Property);
}
#pragma endregion 

#pragma region Text
bool UNeatMetadataCollection_Text::IsRelevantForContainedProperty(const FProperty& InProperty) const
{
	return InProperty.IsA<FNameProperty>() || InProperty.IsA<FStrProperty>() || InProperty.IsA<FTextProperty>();
}
#pragma endregion

#pragma region Map
bool UNeatMetadataCollection_Map::IsRelevantForProperty(const FProperty& InProperty) const
{
	return InProperty.IsA<FMapProperty>();
}
#pragma endregion
