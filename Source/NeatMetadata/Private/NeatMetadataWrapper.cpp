// Copyright Viktor Pramberg. All Rights Reserved.
#include "NeatMetadataWrapper.h"
#include "Kismet2/BlueprintEditorUtils.h"

FNeatMetadataWrapper::FNeatMetadataWrapper(TWeakFieldPtr<FProperty> InProperty, TWeakObjectPtr<UBlueprint> InBlueprint) :
	Property(InProperty),
	Blueprint(InBlueprint),
	VariableDesc(Blueprint->NewVariables.FindByPredicate([this](const FBPVariableDescription& InDesc) { return InDesc.VarName == Property->GetFName(); }))
{
}

void FNeatMetadataWrapper::SetMetadata(FName Key, const FString& Value) const
{
	if (IsValid())
	{
		Blueprint->Modify();
		FBlueprintEditorUtils::SetBlueprintVariableMetaData(Blueprint.Get(), Property->GetFName(), nullptr, Key, Value);
	}
}

void FNeatMetadataWrapper::RemoveMetadata(FName Key) const
{
	if (IsValid())
	{
		Blueprint->Modify();
		FBlueprintEditorUtils::RemoveBlueprintVariableMetaData(Blueprint.Get(), Property->GetFName(), nullptr, Key);
	}
}

FString FNeatMetadataWrapper::GetMetadata(FName Key) const
{
	return IsValid() && VariableDesc->HasMetaData(Key) ? VariableDesc->GetMetaData(Key) : FString();
}

bool FNeatMetadataWrapper::HasMetadata(FName Key) const
{
	return IsValid() ? VariableDesc->HasMetaData(Key) : false;
}

bool FNeatMetadataWrapper::IsValid() const
{
	return Blueprint.IsValid() && Property.IsValid() && VariableDesc;
}

const FProperty* FNeatMetadataWrapper::GetProperty() const
{
	return Property.Get();
}

UBlueprint* FNeatMetadataWrapper::GetBlueprint() const
{
	return Blueprint.Get();
}