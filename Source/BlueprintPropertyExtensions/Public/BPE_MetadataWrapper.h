// Copyright Viktor Pramberg. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/WeakFieldPtr.h"

/**
 * 
 */
class BLUEPRINTPROPERTYEXTENSIONS_API FBPE_MetadataWrapper
{
public:
	FBPE_MetadataWrapper() = default;
	FBPE_MetadataWrapper(TWeakFieldPtr<FProperty> InProperty, TWeakObjectPtr<UBlueprint> InBlueprint);
	void SetMetadata(FName Key, const FString& Value) const;
	void SetOrRemoveMetadata(FName Key, const FString& Value) const;
	void RemoveMetadata(FName Key) const;
	FString GetMetadata(FName Key) const;
	bool HasMetadata(FName Key) const;

	const FProperty* GetProperty() const; 
	UBlueprint* GetBlueprint() const; 
	
private:
	bool GetMetadataContainers(FProperty*& OutProp, FBPVariableDescription*& OutDesc) const;

	TWeakFieldPtr<FProperty> Property;
	TWeakObjectPtr<UBlueprint> Blueprint;
};
