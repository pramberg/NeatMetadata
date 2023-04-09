// Copyright Viktor Pramberg. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/WeakFieldPtr.h"

/**
 * 
 */
class NEATMETADATA_API FNeatMetadataWrapper
{
public:
	FNeatMetadataWrapper() = default;
	FNeatMetadataWrapper(TWeakFieldPtr<FProperty> InProperty, TWeakObjectPtr<UBlueprint> InBlueprint);

	void SetMetadata(FName Key, const FString& Value) const;
	void RemoveMetadata(FName Key) const;
	FString GetMetadata(FName Key) const;
	bool HasMetadata(FName Key) const;

	bool IsValid() const;
	const FProperty* GetProperty() const; 
	UBlueprint* GetBlueprint() const;
	
private:
	TWeakFieldPtr<FProperty> Property = nullptr;
	TWeakObjectPtr<UBlueprint> Blueprint = nullptr;

	// TODO: Is this valid? What happens if the desc is deleted? Can that even happen while this wrapper is valid?
	FBPVariableDescription* VariableDesc = nullptr;
};
