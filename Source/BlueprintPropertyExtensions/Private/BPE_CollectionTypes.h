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
	virtual TOptional<FString> ExportValueForProperty(FProperty& Property) const override;
	virtual void ImportValueForProperty(const FProperty& Property, const FString& Value) override;
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

	// If the edit condition isn't met, should this property be hidden?
	UPROPERTY(EditAnywhere)
	bool EditConditionHides;

	// If this property is used as an edit condition, should it be displayed inline next to the other property?
	// This is only supported for boolean properties.
	UPROPERTY(EditAnywhere)
	bool InlineEditConditionToggle;

	// When the variable in the edit condition has `InlineEditConditionToggle` enabled, should it be hidden on this property?
	// This setting can be useful if you use an inline condition variable to disable multiple properties, but only want the checkbox
	// to be shown on one/some of those properties.
	UPROPERTY(EditAnywhere)
	bool HideEditConditionToggle;

protected:
	virtual bool IsPropertyVisible(const FProperty& Property) const override;
};



/**
 * Controls what unit this property represents. For example, a float variable can represent
 * distance, angles or speed. By specifying a unit, you can make it easier for a user to
 * understand the use of the property.
 */
UCLASS(meta=(DisplayName = "Units"))
class UBPE_MetadataCollection_Units : public UBPE_MetadataCollectionStruct
{
	GENERATED_BODY()

public:
	// Specifies what unit this property represents. The value of the property is automatically converted
	// from the user's preferred unit for that value type. If you specify `Centimeter` and the user's preferred
	// unit is `Yards`, Unreal will handle the conversion from yards to centimeters automatically.
	UPROPERTY(EditAnywhere, meta = (EditCondition = "ForceUnits == EUnit::Unspecified", EditConditionHides))
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
	virtual bool IsRelevantForContainedProperty(const FProperty& InProperty) const override;
	virtual TOptional<FString> ExportValueForProperty(FProperty& Property) const override;
	virtual void ImportValueForProperty(const FProperty& Property, const FString& Value) override;
};



/** Allows you to specify the name of the axes on runtime curve properties. */
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
 * Controls what Asset Bundles this property belongs to.
 * TODO: Improve documentation.
 *
 * @see https://docs.unrealengine.com/5.1/en-US/asset-management-in-unreal-engine/#assetbundles
 * @see UAssetManager::InitializeAssetBundlesFromMetadata
 */
UCLASS(meta=(DisplayName = "Asset Bundles"))
class UBPE_MetadataCollection_AssetBundles : public UBPE_MetadataCollection
{
	GENERATED_BODY()

public:
	// The bundles to add this property to.
	UPROPERTY(EditAnywhere)
	TArray<FString> AssetBundles;

protected:
	virtual bool IsRelevantForContainedProperty(const FProperty& InProperty) const override;
	virtual TOptional<FString> ExportValueForProperty(FProperty& Property) const override;
	virtual void ImportValueForProperty(const FProperty& Property, const FString& Value) override;
};



/** Metadata related to color properties. Currently only allows you to hide the alpha channel. */
UCLASS(meta=(DisplayName = "Color"))
class UBPE_MetadataCollection_Color : public UBPE_MetadataCollectionStruct
{
	GENERATED_BODY()

public:
	UBPE_MetadataCollection_Color();

	// Whether to hide the alpha channel from the color picker.
	UPROPERTY(EditAnywhere)
	bool HideAlphaChannel;
};



/** Controls the format of the header row on array elements. */
UCLASS(meta=(DisplayName = "Title Property", Group = "Array"))
class UBPE_MetadataCollection_TitleProperty : public UBPE_MetadataCollection
{
	GENERATED_BODY()

public:
	// Determines the format of the header on each array element.
	// You may specify a single property like this: "SomePropertyInStruct".
	// You may also specify a Text-like formatting: "{SomePropertyInStruct} - {SomeOtherPropertyInStruct}".
	UPROPERTY(EditAnywhere)
	FString TitleProperty;

protected:
	virtual bool IsRelevantForProperty(const FProperty& InProperty) const override;
	virtual bool IsRelevantForContainedProperty(const FProperty& InProperty) const override;
};



/** Exposes the possibility to specify a list of strings as an option to String or Name variables. */
UCLASS(meta=(DisplayName = "Get Options", Group = "General"))
class UBPE_MetadataCollection_GetOptions : public UBPE_MetadataCollectionStruct
{
	GENERATED_BODY()

public:
	// The name of a function that produces the array of Strings/Names to be used as options.
	//
	// There are a few caveats:
	// * The function can be either a function inside the Blueprint, or a static C++ function.
	// * The function CANNOT be defined in a BlueprintFunctionLibrary asset. C++ BlueprintFunctionLibraries are still okay.
	// * The function needs to return a single array of Strings or Names.
	// * The name of the output should be called "ReturnValue".
	// * The function may not take any input parameters.
	UPROPERTY(EditAnywhere)
	FString GetOptions;

protected:
	virtual TSharedPtr<SWidget> CreateValueWidgetForProperty(const TSharedRef<IPropertyHandle>& InHandle) override;
	TOptional<FString> ValidateOptionsFunction(const FString& FunctionName) const;
	virtual TOptional<FString> ExportValueForProperty(FProperty& Property) const override;
	virtual bool IsRelevantForContainedProperty(const FProperty& InProperty) const override;

private:
	// Decides whether or not the function generated by `AddNewFunction` will return names or strings.
	// Mutable because the easiest place to check this is in `IsRelevantForContainedProperty`.
	mutable bool bIsString = true;
	
	TOptional<FString> OnAddNewFunction() const;

	// Functions used to easily create new function graphs with the correct signature.
	UFUNCTION() static TArray<FString> GetOptionsStringSignature() { return {}; }
	UFUNCTION() static TArray<FName> GetOptionsNameSignature() { return {}; }
};



/** Exposes the possibility to specify a list of strings as an option to String or Name variables. */
UCLASS(meta=(DisplayName = "Directory Path", Group = "Paths"))
class UBPE_MetadataCollection_DirectoryPath : public UBPE_MetadataCollectionStruct
{
	GENERATED_BODY()

public:
	UBPE_MetadataCollection_DirectoryPath();

	UPROPERTY(EditAnywhere)
	bool RelativePath;

	UPROPERTY(EditAnywhere)
	bool ContentDir;

	UPROPERTY(EditAnywhere)
	bool RelativeToGameContentDir;

	UPROPERTY(EditAnywhere)
	bool LongPackageName;
};



USTRUCT()
struct FBPE_FilePathFilter
{
	GENERATED_BODY()

	// The file extension to filter for. Does not need "*.".
	UPROPERTY(EditAnywhere)
	FString Extension;

	// Optional description of this filter. If empty this will be generated from the extension.
	UPROPERTY(EditAnywhere)
	FString Description;
};

/** Exposes the possibility to specify a list of strings as an option to String or Name variables. */
UCLASS(meta=(DisplayName = "File Path", Group = "Paths"))
class UBPE_MetadataCollection_FilePath : public UBPE_MetadataCollectionStruct
{
	GENERATED_BODY()

public:
	UBPE_MetadataCollection_FilePath();

	UPROPERTY(EditAnywhere)
	bool RelativeToGameDir;

	// The File Path Filter describes what file types should show up in the file picker.
	UPROPERTY(EditAnywhere, DisplayName = "File Path Filter", meta = (TitleProperty = "{Description} (*.{Extension})"))
	TArray<FBPE_FilePathFilter> FilePathFilter_Internal;

protected:
	virtual TOptional<FString> ExportValueForProperty(FProperty& Property) const override;
	void SetFilePathFilterMetadata() const;
};



/** Exposes the possibility to specify a list of strings as an option to String or Name variables. */
UCLASS(meta=(DisplayName = "Primary Asset Id", Group = "General"))
class UBPE_MetadataCollection_PrimaryAssetId : public UBPE_MetadataCollectionStruct
{
	GENERATED_BODY()

public:
	UBPE_MetadataCollection_PrimaryAssetId();

	UPROPERTY(EditAnywhere)
	TSet<FPrimaryAssetType> AllowedTypes;

protected:
	virtual TOptional<FString> ExportValueForProperty(FProperty& Property) const override;
	virtual void ImportValueForProperty(const FProperty& Property, const FString& Value) override;
};



/** Whether to remove the parent scope around the struct property. */
UCLASS(meta=(DisplayName = "Show Only Inner Properties", Group = "General"))
class UBPE_MetadataCollection_ShowOnlyInnerProperties : public UBPE_MetadataCollection
{
	GENERATED_BODY()

public:
	// Removes the parent scope around this struct and displays all of this struct's properties inline.
	UPROPERTY(EditAnywhere)
	bool ShowOnlyInnerProperties = false;

protected:
	virtual bool IsRelevantForContainedProperty(const FProperty& InProperty) const override;
};



/** Options for class and soft class properties. Allows you to narrow down what classes are allowed to be selected. */
UCLASS(meta=(DisplayName = "Class Picker"))
class UBPE_MetadataCollection_ClassPicker : public UBPE_MetadataCollection
{
	GENERATED_BODY()

public:
	// Whether to allow abstract classes to be selected.
	UPROPERTY(EditAnywhere)
	bool AllowAbstract = false;

	// Whether to show the classes in a tree view or a list view.
	UPROPERTY(EditAnywhere)
	bool ShowTreeView = false;

	// Whether to only allow classes created by blueprints, or allow native classes too.
	UPROPERTY(EditAnywhere)
	bool BlueprintBaseOnly = false;

	// Whether to only allow actors to be selected, or any object.
	UPROPERTY(EditAnywhere)
	bool OnlyPlaceable = false;

	// Whether to allow the "Create New" button that appears next to the property widget by default.
	UPROPERTY(EditAnywhere)
	bool DisallowCreateNew = false;

	// Whether to show the exact name of classes, or use their display name.
	UPROPERTY(EditAnywhere)
	bool ShowDisplayNames = false;

	// Whether to hide the view options button in the menu.
	UPROPERTY(EditAnywhere)
	bool HideViewOptions = false;

	// Specifies an interface that classes must implement in order to be selectable.
	UPROPERTY(EditAnywhere)
	UClass* MustImplement = nullptr;

protected:
	virtual bool IsRelevantForContainedProperty(const FProperty& InProperty) const override;
	virtual TSharedPtr<SWidget> CreateValueWidgetForProperty(const TSharedRef<IPropertyHandle>& InHandle) override;
};



/** Some arcane and obscure array-specific metadata. */
UCLASS(meta=(DisplayName = "Array", Group = "Array"))
class UBPE_MetadataCollection_Array : public UBPE_MetadataCollection
{
	GENERATED_BODY()

public:
	// Removes the possibility to use Duplicate on an element.
	UPROPERTY(EditAnywhere)
	bool NoElementDuplicate = false;

	// Removes the possibility to reorder elements in the array.
	UPROPERTY(EditAnywhere)
	bool EditFixedOrder = false;

protected:
	virtual bool IsRelevantForProperty(const FProperty& InProperty) const override;
};

/** Small extensions to numeric properties. */
UCLASS(meta=(DisplayName = "Numbers"))
class UBPE_MetadataCollection_Numbers : public UBPE_MetadataCollection
{
	GENERATED_BODY()

public:
	// If true, removes the possibility to drag the widget. The user is then only allowed to write values into the box.
	UPROPERTY(EditAnywhere)
	bool NoSpinbox = false;

	// Use exponential scale for the slider.
	UPROPERTY(EditAnywhere, meta = (EditCondition = "!NoSpinbox", EditConditionHides))
	float SliderExponent = 1.0f;

	// Delta to increment the value as the slider moves. If not specified it will be determined by the spin box.
	UPROPERTY(EditAnywhere, meta = (EditCondition = "!NoSpinbox", EditConditionHides))
	float Delta = 0.0f;

	// If we're an unbounded spin box, what value do we divide mouse movement by before multiplying by Delta. Requires Delta to be set.
	UPROPERTY(EditAnywhere, meta = (EditCondition = "!NoSpinbox && Delta > 0.0", EditConditionHides))
	float LinearDeltaSensitivity = 0.0f;

	// How much to increment this value when the user is scrolling with their mouse wheel.
	UPROPERTY(EditAnywhere, meta = (EditCondition = "!NoSpinbox", EditConditionHides))
	float WheelStep = 0.0f;
	
	// This property is only allowed to be a multiple of this value. Value will be floored to the closest multiple.
	UPROPERTY(EditAnywhere)
	float Multiple = 1.0f;

	// The name of an array property that this property will be clamped to.
	UPROPERTY(EditAnywhere, meta = (GetOptions = "GetAllArrayProperties"))
	FString ArrayClamp;

protected:
	UFUNCTION()
	TArray<FString> GetAllArrayProperties() const;
	virtual TOptional<FString> ExportValueForProperty(FProperty& Property) const override;
	virtual bool IsRelevantForContainedProperty(const FProperty& InProperty) const override;
	virtual bool IsPropertyVisible(const FProperty& Property) const override;
};