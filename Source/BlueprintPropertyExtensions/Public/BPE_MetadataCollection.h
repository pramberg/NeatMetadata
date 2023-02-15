// Copyright Viktor Pramberg. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Math/UnitConversion.h"
#include "BPE_MetadataCollection.generated.h"

class FBPE_MetadataWrapper;

/**
 * 
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class BLUEPRINTPROPERTYEXTENSIONS_API UBPE_MetadataCollection : public UObject
{
	GENERATED_BODY()

public:
	UBPE_MetadataCollection();
	virtual TSharedRef<SWidget> CreateWidget(const FBPE_MetadataWrapper& MetadataWrapper);
	virtual bool IsRelevantForProperty(FProperty* InProperty) const;
	virtual TOptional<FText> GetGroup() const;
	virtual bool IsPropertyVisible(FName PropertyName);
	virtual void InitializeFromMetadata(const FBPE_MetadataWrapper& MetadataWrapper);
	virtual void OnPropertyChanged(const FPropertyChangedEvent& PropChanged, FBPE_MetadataWrapper MetadataWrapper);

protected:
	virtual FString GetValueForProperty(FProperty& Property) const;
	virtual void SetValueForProperty(const FProperty& Property, const FString& Value);
	virtual void InitializeValueForProperty(const FProperty& Property);
	
protected:
	FFieldClass* PropertyClass;
};

UCLASS(Abstract)
class BLUEPRINTPROPERTYEXTENSIONS_API UBPE_MetadataCollectionStruct : public UBPE_MetadataCollection
{
	GENERATED_BODY()

public:
	virtual bool IsRelevantForProperty(FProperty* InProperty) const override;
	
protected:
	UPROPERTY(EditDefaultsOnly)
	TArray<TObjectPtr<UScriptStruct>> Structs;
};