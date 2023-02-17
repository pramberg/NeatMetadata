// Copyright Viktor Pramberg. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
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
	
	void Setup(const FBPE_MetadataWrapper& MetadataWrapper);
	
	using FForEachVisiblePropertySignature = void(const FProperty&);
	void ForEachVisibleProperty(TFunctionRef<FForEachVisiblePropertySignature> Functor);

	virtual bool IsRelevantForProperty(FProperty* InProperty) const;
	virtual TOptional<FText> GetGroup() const;
	virtual void OnPropertyChanged(const FPropertyChangedEvent& PropChanged, FBPE_MetadataWrapper MetadataWrapper);

protected:
	static inline TOptional<FString> NoPropertyValue = TOptional<FString>();
	
	virtual TOptional<FString> GetValueForProperty(FProperty& Property) const;
	virtual void SetValueForProperty(const FProperty& Property, const FString& Value);
	virtual void InitializeValueForProperty(const FProperty& Property);
	virtual bool IsPropertyVisible(const FProperty& Property);

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