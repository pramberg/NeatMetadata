// Copyright Viktor Pramberg. All Rights Reserved.


#include "BPE_MetadataWrapper.h"

FBPE_MetadataWrapper::FBPE_MetadataWrapper(TWeakFieldPtr<FProperty> InProperty, TWeakObjectPtr<UBlueprint> InBlueprint): Property(InProperty), Blueprint(InBlueprint)
{
}

void FBPE_MetadataWrapper::SetMetadata(FName Key, const FString& Value) const
{
	FProperty* Prop;
	FBPVariableDescription* Desc;
	if (GetMetadataContainers(Prop, Desc))
	{
		Prop->SetMetaData(Key, *Value);
		Desc->SetMetaData(Key,Value);
	}
}

void FBPE_MetadataWrapper::SetOrRemoveMetadata(FName Key, const FString& Value) const
{
	if (Value.IsEmpty())
	{
		RemoveMetadata(Key);
	}
	else
	{
		SetMetadata(Key, Value);
	}
}

void FBPE_MetadataWrapper::RemoveMetadata(FName Key) const
{
	FProperty* Prop;
	FBPVariableDescription* Desc;
	if (GetMetadataContainers(Prop, Desc))
	{
		Prop->RemoveMetaData(Key);
		Desc->RemoveMetaData(Key);
	}
}

FString FBPE_MetadataWrapper::GetMetadata(FName Key) const
{
	FProperty* Prop;
	FBPVariableDescription* Desc;
	if (GetMetadataContainers(Prop, Desc))
	{
		return Desc->HasMetaData(Key) ? Desc->GetMetaData(Key) : FString();
	}
	return FString();
}

bool FBPE_MetadataWrapper::HasMetadata(FName Key) const
{
	FProperty* Prop;
	FBPVariableDescription* Desc;
	if (GetMetadataContainers(Prop, Desc))
	{
		return Desc->HasMetaData(Key);
	}
	return false;
}

bool FBPE_MetadataWrapper::GetMetadataContainers(FProperty*& OutProp, FBPVariableDescription*& OutDesc) const
{
	if (!Blueprint.IsValid() || !Property.IsValid())
	{
		return false;
	}
			
	UBlueprint* BP = Blueprint.Get();
	FProperty* Prop = Property.Get();

	FBPVariableDescription* Desc = BP->NewVariables
		.FindByPredicate([Prop](const FBPVariableDescription& InDesc) { return InDesc.VarName == Prop->GetFName(); });
			
	if (!Desc)
	{
		return false;
	}

	OutProp = Property.Get();
	OutDesc = Desc;

	return true;
}
