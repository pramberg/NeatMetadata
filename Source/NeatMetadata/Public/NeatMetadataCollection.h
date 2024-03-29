// Copyright Viktor Pramberg. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NeatMetadataWrapper.h"
#include "NeatMetadataCollection.generated.h"

/**
 * Encapsulates a collection of metadata for some blueprint variable. UPROPERTIES in this class that are visible in
 * the editor (i.e. EditAnywhere) will show up in the details panel of all selected variables that matches its conditions.
 *
 * Instances of these objects are created and reused in order to display their properties.
 * @see UNeatMetadataSettings.
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class NEATMETADATA_API UNeatMetadataCollection : public UObject
{
	GENERATED_BODY()

public:
	UNeatMetadataCollection();
	
	/**
	 * @brief Sets this object up for display based on the provided metadata.
	 * @param MetadataWrapper The metadata to apply to this object.
	 */
	void InitializeFromMetadata(const FNeatMetadataWrapper& MetadataWrapper);
	
	using FForEachVisiblePropertySignature = void(const FProperty&);
	/**
	 * @brief Loops through all visible properties on this object.
	 * @param Functor Functor that executes for each property.
	 */
	void ForEachVisibleProperty(TFunctionRef<FForEachVisiblePropertySignature> Functor) const;
	
	/**
	 * @brief Is this collection relevant for the input property.
	 * @param InProperty The property representing the blueprint variable. This is the root property, if it's an array, this will be an FArrayProperty.
	 * @return True if we should show this collection for this property.
	 */
	virtual bool IsRelevantForProperty(const FProperty& InProperty) const;
	
	/**
	 * @brief May return a widget if the collection wants to provide a custom widget for a property.
	 * @param InHandle A property handle that may want a custom display.
	 * @return A widget or a nullptr if no custom display is desired.
	 */
	virtual TSharedPtr<SWidget> CreateValueWidgetForProperty(const TSharedRef<IPropertyHandle>& InHandle);

protected:
	/**
	 * @brief Is this collection relevant for the potentially *contained* input property.
	 * @param InProperty Either the actual property, or if the root property is a container, this represents the inner property.
	 * @return True if we should show this collection for this property.
	 */
	virtual bool IsRelevantForContainedProperty(const FProperty& InProperty) const;

	/**
	 * @brief Should the input property be visible in the UI?
	 * @param Property The property to test. This is a member property of this object.
	 * @return True if it is visible.
	 */
	virtual bool IsPropertyVisible(const FProperty& Property) const;
	
	/**
	 * @brief Retrieves the value for the input property so that it can be stored as metadata.
	 * @param Property The property on this object to retrieve the value from.
	 * @return The value if you want to set it, or `NullOpt` if you want to remove the metadata.
	 */
	virtual TOptional<FString> ExportValueForProperty(FProperty& Property) const;
	
	/**
	 * @brief Imports the value of a property to this object from a string.
	 * @param Property The property on this object to set the value on.
	 * @param Value The value to set the property to, that has been retrieved from metadata.
	 */
	virtual void ImportValueForProperty(const FProperty& Property, const FString& Value);
	
	/**
	 * @brief Initializes the value for a property to its defaults, from the CDO.
	 * @param Property The property to initialize.
	 */
	void InitializeValueForProperty(const FProperty& Property);

protected:
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

protected:
	FNeatMetadataWrapper CurrentWrapper;
};


/**
 * A version of a metadata collection that is only relevant for struct properties with specified struct types.
 */
UCLASS(Abstract)
class NEATMETADATA_API UNeatMetadataCollectionStruct : public UNeatMetadataCollection
{
	GENERATED_BODY()

public:
	virtual bool IsRelevantForContainedProperty(const FProperty& InProperty) const override;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Metadata Collection")
	TArray<TObjectPtr<UScriptStruct>> Structs;
};