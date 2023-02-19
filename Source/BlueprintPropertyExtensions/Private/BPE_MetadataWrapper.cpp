// Copyright Viktor Pramberg. All Rights Reserved.
#include "BPE_MetadataWrapper.h"
#include "Kismet2/BlueprintEditorUtils.h"

FBPE_MetadataWrapper::FBPE_MetadataWrapper(TWeakFieldPtr<FProperty> InProperty, TWeakObjectPtr<UBlueprint> InBlueprint) :
	Property(InProperty),
	Blueprint(InBlueprint),
	VariableDesc(Blueprint->NewVariables.FindByPredicate([this](const FBPVariableDescription& InDesc) { return InDesc.VarName == Property->GetFName(); }))
{
	check(IsValid());
}

void FBPE_MetadataWrapper::SetMetadata(FName Key, const FString& Value) const
{
	if (IsValid())
	{
		Blueprint->Modify();
		FBlueprintEditorUtils::SetBlueprintVariableMetaData(Blueprint.Get(), Property->GetFName(), nullptr, Key, Value);
	}
}

void FBPE_MetadataWrapper::RemoveMetadata(FName Key) const
{
	if (IsValid())
	{
		Blueprint->Modify();
		FBlueprintEditorUtils::RemoveBlueprintVariableMetaData(Blueprint.Get(), Property->GetFName(), nullptr, Key);
	}
}

FString FBPE_MetadataWrapper::GetMetadata(FName Key) const
{
	return IsValid() && VariableDesc->HasMetaData(Key) ? VariableDesc->GetMetaData(Key) : FString();
}

bool FBPE_MetadataWrapper::HasMetadata(FName Key) const
{
	return IsValid() ? VariableDesc->HasMetaData(Key) : false;
}

bool FBPE_MetadataWrapper::IsValid() const
{
	return Blueprint.IsValid() && Property.IsValid() && VariableDesc;
}

const FProperty* FBPE_MetadataWrapper::GetProperty() const
{
	return Property.Get();
}

UBlueprint* FBPE_MetadataWrapper::GetBlueprint() const
{
	return Blueprint.Get();
}