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

	virtual void InitializeFromMetadata(const FBPE_MetadataWrapper& MetadataWrapper);
	virtual FString GetValueForProperty(FProperty& Property) const;
	virtual void SetValueForProperty(const FProperty& Property, const FString& Value);
	virtual void InitializeValueForProperty(const FProperty& Property);
	virtual void OnPropertyChanged(const FPropertyChangedEvent& PropChanged, FBPE_MetadataWrapper MetadataWrapper);
	virtual bool IsPropertyVisible(FName PropertyName);
	
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

UCLASS(meta=(DisplayName = "Gameplay Tag Categories"))
class UBPE_MetadataCollection_GameplayTagCategories : public UBPE_MetadataCollectionStruct
{
	GENERATED_BODY()

public:
	UBPE_MetadataCollection_GameplayTagCategories();
	virtual FString GetValueForProperty(FProperty& Property) const override;
	virtual void SetValueForProperty(const FProperty& Property, const FString& Value) override;
	
public:
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer Categories;
};

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