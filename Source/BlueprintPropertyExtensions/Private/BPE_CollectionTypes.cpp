// Copyright Viktor Pramberg. All Rights Reserved.
#include "BPE_CollectionTypes.h"
#include "BPE_Module.h"

#include "Widgets/SBPE_InterfaceSelector.h"
#include "Widgets/SBPE_FunctionSelector.h"

#include "Curves/CurveLinearColor.h"
#include "Curves/CurveVector.h"

#include "Kismet2/BlueprintEditorUtils.h"
#include "BlueprintEditorModule.h"

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
bool UBPE_MetadataCollection_EditCondition::IsPropertyVisible(const FProperty& Property) const
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
UBPE_MetadataCollection_GameplayTagCategories::UBPE_MetadataCollection_GameplayTagCategories()
{
	Structs = {
		FGameplayTag::StaticStruct(),
		FGameplayTagContainer::StaticStruct(),
	};
}

TOptional<FString> UBPE_MetadataCollection_GameplayTagCategories::ExportValueForProperty(FProperty& Property) const
{
	if (Property.GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, Categories))
	{
		// ToStringSimple adds a ", ". The space causes issues when parsing multiple tags...
		const FString Result = Categories.ToStringSimple().Replace(TEXT(" "), TEXT(""));
		return Result.IsEmpty() ? NullOpt : TOptional(Result);
	}
	
	return Super::ExportValueForProperty(Property);
}

void UBPE_MetadataCollection_GameplayTagCategories::ImportValueForProperty(const FProperty& Property, const FString& Value)
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
bool UBPE_MetadataCollection_Units::IsRelevantForContainedProperty(const FProperty& InProperty) const
{
	return IsNumericProperty(InProperty);
}

TOptional<FString> UBPE_MetadataCollection_Units::ExportValueForProperty(FProperty& Property) const
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

void UBPE_MetadataCollection_Units::ImportValueForProperty(const FProperty& Property, const FString& Value)
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

TOptional<FString> UBPE_MetadataCollection_AssetBundles::ExportValueForProperty(FProperty& Property) const
{
	if (Property.GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, AssetBundles))
	{
		const FString Result = FString::Join(AssetBundles, TEXT(","));
		return AssetBundles.IsEmpty() ? NullOpt : TOptional(Result);
	}
	
	return Super::ExportValueForProperty(Property);
}

void UBPE_MetadataCollection_AssetBundles::ImportValueForProperty(const FProperty& Property, const FString& Value)
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

TSharedPtr<SWidget> UBPE_MetadataCollection_GetOptions::CreateValueWidgetForProperty(const TSharedRef<IPropertyHandle>& InHandle)
{
	if (InHandle->GetProperty()->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, GetOptions))
	{
		return SNew(SBPE_FunctionSelector, InHandle)
		.FunctionFilter_Static(&GetOptionsFunctionFilter)
		.AddNewFunction_UObject(this, &ThisClass::OnAddNewFunction)
		.MemberClass(CurrentWrapper.GetProperty()->GetOwnerClass());
	}
	
	return Super::CreateValueWidgetForProperty(InHandle);
}

TOptional<FString> UBPE_MetadataCollection_GetOptions::ValidateOptionsFunction(const FString& FunctionName) const
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

TOptional<FString> UBPE_MetadataCollection_GetOptions::ExportValueForProperty(FProperty& Property) const
{
	if (Property.GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, GetOptions))
	{
		if (TOptional<FString> ErrorString = ValidateOptionsFunction(GetOptions))
		{
			// TODO: Figure out how to log this inside the Blueprint editor...
			UE_LOG(LogBlueprintPropertyExtensions, Error, TEXT("GetOptions[%s]: %s"), *CurrentWrapper.GetProperty()->GetName(), *ErrorString.GetValue());
			return {};
		}
	}
	
	return Super::ExportValueForProperty(Property);
}

bool UBPE_MetadataCollection_GetOptions::IsRelevantForContainedProperty(const FProperty& InProperty) const
{
	bIsString = InProperty.IsA<FStrProperty>();
	return bIsString || InProperty.IsA<FNameProperty>();
}

TOptional<FString> UBPE_MetadataCollection_GetOptions::OnAddNewFunction() const
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
UBPE_MetadataCollection_DirectoryPath::UBPE_MetadataCollection_DirectoryPath()
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
UBPE_MetadataCollection_FilePath::UBPE_MetadataCollection_FilePath()
{
	// FFilePath doesn't have a StaticStruct() member nor a TBaseStructure<FFilePath>::Get() implementation.
	// Therefore have to find it manually.
	static UScriptStruct* FilePathStruct = FindObjectChecked<UScriptStruct>(UObject::StaticClass()->GetPackage(), TEXT("FilePath"));
	Structs = {
		FilePathStruct
	};
}

TOptional<FString> UBPE_MetadataCollection_FilePath::ExportValueForProperty(FProperty& Property) const
{
	if (Property.GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, FilePathFilter_Internal))
	{
		SetFilePathFilterMetadata();
	}
	
	return Super::ExportValueForProperty(Property);
}

void UBPE_MetadataCollection_FilePath::SetFilePathFilterMetadata() const
{
	// The setup for FilePathFilter is a bit more complicated than typical.
	// We could either parse the metadata string like what happens in trivial cases where we just need a slightly different
	// format on the string. But due to the potentially complex format of the filter string, I prefer to store the raw
	// filter "description" in a separate field, and generate the real filter based on that description. This solution vastly
	// reduces the complexity, but of course introduces a new metadata field. I think that's a fair compromise, since
	// property metadata is just an editor feature anyways.
	static const FName FilePathFilter("FilePathFilter");
	
	TArray<FBPE_FilePathFilter> FilePathDescCopy = FilePathFilter_Internal;
	FilePathDescCopy.RemoveAll([](const FBPE_FilePathFilter& Filter) { return Filter.Extension.IsEmpty(); });
	
	if (FilePathDescCopy.IsEmpty())
	{
		CurrentWrapper.RemoveMetadata(FilePathFilter);
		return;
	}
		
	const FString IndividualFiles = FString::JoinBy(FilePathDescCopy, TEXT("|"), [](const FBPE_FilePathFilter& Filter)
	{
		const FString Description = Filter.Description.IsEmpty() ? FString::Printf(TEXT("%s Files"), *Filter.Extension.ToUpper()) : Filter.Description;
		return FString::Printf(TEXT("%s (*.%s)|*.%s"), *Description, *Filter.Extension, *Filter.Extension);
	});

	if (FilePathDescCopy.Num() == 1)
	{
		CurrentWrapper.SetMetadata(FilePathFilter, IndividualFiles);
		return;
	}
		
	const FString CombinedFiles = FString::JoinBy(FilePathDescCopy, TEXT(";"), [](const FBPE_FilePathFilter& Filter)
	{
		return FString::Printf(TEXT("*.%s"), *Filter.Extension);
	});

	CurrentWrapper.SetMetadata(FilePathFilter, FString::Printf(TEXT("All Files (%s)|%s|%s"), *CombinedFiles, *CombinedFiles, *IndividualFiles));
}
#pragma endregion

#pragma region Primary Asset Id
UBPE_MetadataCollection_PrimaryAssetId::UBPE_MetadataCollection_PrimaryAssetId()
{
	Structs = {
		TBaseStructure<FPrimaryAssetId>::Get(),
	};
}

TOptional<FString> UBPE_MetadataCollection_PrimaryAssetId::ExportValueForProperty(FProperty& Property) const
{
	if (Property.GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, AllowedTypes))
	{
		const FString Result = FString::JoinBy(AllowedTypes, TEXT(","), [](const FPrimaryAssetType& Type) { return Type.ToString(); });
		return AllowedTypes.IsEmpty() ? NullOpt : TOptional(Result);
	}
	
	return Super::ExportValueForProperty(Property);
}

void UBPE_MetadataCollection_PrimaryAssetId::ImportValueForProperty(const FProperty& Property, const FString& Value)
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
bool UBPE_MetadataCollection_ShowOnlyInnerProperties::IsRelevantForContainedProperty(const FProperty& InProperty) const
{
	return InProperty.IsA<FStructProperty>();
}
#pragma endregion

#pragma region Class Picker
bool UBPE_MetadataCollection_ClassPicker::IsRelevantForContainedProperty(const FProperty& InProperty) const
{
	return InProperty.IsA<FSoftClassProperty>() || InProperty.IsA<FClassProperty>();
}

TSharedPtr<SWidget> UBPE_MetadataCollection_ClassPicker::CreateValueWidgetForProperty(const TSharedRef<IPropertyHandle>& InHandle)
{
	if (InHandle->GetProperty()->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, MustImplement))
	{
		return SNew(SBPE_InterfaceSelector, InHandle);
	}
	
	return Super::CreateValueWidgetForProperty(InHandle);
}
#pragma endregion


#pragma region Array
bool UBPE_MetadataCollection_Array::IsRelevantForProperty(const FProperty& InProperty) const
{
	return InProperty.IsA<FArrayProperty>();
}
#pragma endregion

#pragma region Numbers
TArray<FString> UBPE_MetadataCollection_Numbers::GetAllArrayProperties() const
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

TOptional<FString> UBPE_MetadataCollection_Numbers::ExportValueForProperty(FProperty& Property) const
{
	if (Property.GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, ArrayClamp))
	{
		return (ArrayClamp.IsEmpty() || ArrayClamp == TEXT("None")) ? NullOpt : TOptional(ArrayClamp);
	}
	
	return Super::ExportValueForProperty(Property);
}

bool UBPE_MetadataCollection_Numbers::IsRelevantForContainedProperty(const FProperty& InProperty) const
{
	return IsNumericProperty(InProperty);
}

bool UBPE_MetadataCollection_Numbers::IsPropertyVisible(const FProperty& Property) const
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
		check(AsNumeric);
		return AsNumeric->IsInteger();
	}
	
	return true;
}
#pragma endregion
