// Copyright Viktor Pramberg. All Rights Reserved.
#include "BPE_CollectionTypes.h"
#include "BPE_Module.h"
#include "ClassViewerFilter.h"
#include "DetailLayoutBuilder.h"
#include "PropertyCustomizationHelpers.h"
#include "SComponentClassCombo.h"

#include "Curves/CurveLinearColor.h"
#include "Curves/CurveVector.h"
#include "Kismet2/BlueprintEditorUtils.h"

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
		return Result.IsEmpty() ? NoPropertyValue : Result;
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
	// It seems like enum properties created in BP are byte properties, not enum properties...
	if (const FByteProperty* AsByte = CastField<FByteProperty>(&InProperty))
	{
		return !AsByte->IsEnum();
	}
	
	return InProperty.IsA<FNumericProperty>() && !InProperty.IsA<FEnumProperty>();
}

TOptional<FString> UBPE_MetadataCollection_Units::ExportValueForProperty(FProperty& Property) const
{
	if (Property.GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, Units) && Units == EUnit::Unspecified)
	{
		return NoPropertyValue;
	}
	
	if (Property.GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, ForceUnits) && ForceUnits == EUnit::Unspecified)
	{
		return NoPropertyValue;
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
		return AssetBundles.IsEmpty() ? NoPropertyValue : FString::Join(AssetBundles, TEXT(","));
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

TOptional<FString> UBPE_MetadataCollection_GetOptions::ExportValueForProperty(FProperty& Property) const
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
	
	return Super::ExportValueForProperty(Property);
}

bool UBPE_MetadataCollection_GetOptions::IsRelevantForContainedProperty(const FProperty& InProperty) const
{
	return InProperty.IsA<FStrProperty>() || InProperty.IsA<FNameProperty>();
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
		return AllowedTypes.IsEmpty() ? NoPropertyValue : FString::JoinBy(AllowedTypes, TEXT(","), [](const FPrimaryAssetType& Type) { return Type.ToString(); });
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

namespace
{
	// Class filter that only shows interfaces.
	class FBPE_InterfaceClassFilter : public IClassViewerFilter
	{
	public:
		virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
		{
			return InClass->HasAnyClassFlags(EClassFlags::CLASS_Interface);
		}

		virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef<const IUnloadedBlueprintData> InClass, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
		{
			return InClass->HasAnyClassFlags(EClassFlags::CLASS_Interface);
		}
	};

	// I wasn't able to find a way to get default class pickers to show all interfaces. That's what this widget does.
	class SBPE_InterfaceClassSelector : public SCompoundWidget
	{
		SLATE_BEGIN_ARGS(SBPE_InterfaceClassSelector) {}
		SLATE_END_ARGS()
		
		void Construct(const FArguments&, TSharedRef<IPropertyHandle> InPropertyHandle)
		{
			PropertyHandle = InPropertyHandle;

			ChildSlot
			[
				SNew(SComboButton)
				.OnGetMenuContent(this, &SBPE_InterfaceClassSelector::GetMenuContent)
				.ButtonContent()
				[
					SNew(STextBlock)
					.Text(this, &SBPE_InterfaceClassSelector::GetButtonText)
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			];
		}

		UClass* GetClass() const
		{
			UObject* Obj;
			PropertyHandle->GetValue(Obj);
			return Cast<UClass>(Obj);
		}

		TSharedRef<SWidget> GetMenuContent()
		{
			FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");
			FClassViewerInitializationOptions Options;
			Options.bShowUnloadedBlueprints = true;
			Options.bShowNoneOption = true;
			Options.bAllowViewOptions = false;
			Options.NameTypeToDisplay = EClassViewerNameTypeToDisplay::DisplayName;
			Options.InitiallySelectedClass = GetClass();
			Options.ClassFilters.Add(MakeShared<FBPE_InterfaceClassFilter>());
					
			return SNew(SBox)
				.WidthOverride(280)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.AutoHeight()
					.MaxHeight(500)
					[
						ClassViewerModule.CreateClassViewer(Options, FOnClassPicked::CreateSP(this, &SBPE_InterfaceClassSelector::OnClassPicked))
					]
				];
		}

		void OnClassPicked(UClass* InClass) const
		{
			if (GetClass() != InClass)
			{
				PropertyHandle->SetValue(InClass);
			}
		}

		FText GetButtonText() const
		{
			return GetClass() ? GetClass()->GetDisplayNameText() : INVTEXT("None");
		}

	private:
		TSharedPtr<IPropertyHandle> PropertyHandle;
	};
}

TSharedPtr<SWidget> UBPE_MetadataCollection_ClassPicker::CreateValueWidgetForProperty(const TSharedRef<IPropertyHandle>& InHandle)
{
	if (InHandle->GetProperty()->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, MustImplement))
	{
		return SNew(SBPE_InterfaceClassSelector, InHandle);
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
