// Copyright Viktor Pramberg. All Rights Reserved.
#include "BPE_MetadataCollection.h"
#include "BPE_MetadataWrapper.h"

UBPE_MetadataCollection::UBPE_MetadataCollection()
{
}

bool UBPE_MetadataCollection::IsRelevantForContainedProperty(const FProperty& InProperty) const
{
	return true;
}

void UBPE_MetadataCollection::InitializeFromMetadata(const FBPE_MetadataWrapper& MetadataWrapper)
{
	CurrentWrapper = MetadataWrapper;
	
	for (const auto* Property : TFieldRange<FProperty>(GetClass()))
	{
		if (Property->HasAnyPropertyFlags(CPF_DisableEditOnInstance))
			continue;

		if (MetadataWrapper.HasMetadata(Property->GetFName()))
		{
			const FString Value = MetadataWrapper.GetMetadata(Property->GetFName());
			SetValueForProperty(*Property, Value);
		}
		else
		{
			InitializeValueForProperty(*Property);
		}
	}
}

void UBPE_MetadataCollection::ForEachVisibleProperty(TFunctionRef<FForEachVisiblePropertySignature> Functor)
{
	for (const FProperty* Prop : TFieldRange<FProperty>(GetClass()))
	{
		if (!IsPropertyVisible(*Prop))
		{
			continue;
		}

		Functor(*Prop);
	}
}

bool UBPE_MetadataCollection::IsRelevantForProperty(const FProperty& InProperty) const
{
	if (const FArrayProperty* AsArray = CastField<FArrayProperty>(&InProperty))
	{
		return AsArray->Inner ? IsRelevantForContainedProperty(*AsArray->Inner) : false;
	}
	
	if (const FMapProperty* AsMap = CastField<FMapProperty>(&InProperty))
	{
		const bool bIsValidKeyProp = AsMap->GetKeyProperty() ? IsRelevantForContainedProperty(*AsMap->GetKeyProperty()) : false;;
		const bool bIsValidValueProp = AsMap->GetValueProperty() ? IsRelevantForContainedProperty(*AsMap->GetValueProperty()) : false;;
		return bIsValidKeyProp || bIsValidValueProp;
	}
	
	if (const FSetProperty* AsSet = CastField<FSetProperty>(&InProperty))
	{
		return AsSet->ElementProp ? IsRelevantForContainedProperty(*AsSet->ElementProp) : false;
	}

	return IsRelevantForContainedProperty(InProperty);
}

TOptional<FString> UBPE_MetadataCollection::GetValueForProperty(FProperty& Property) const
{
	if (const FBoolProperty* AsBool = CastField<FBoolProperty>(&Property))
	{
		if (AsBool->GetPropertyValue_InContainer(this))
		{
			return FString();
		}
		return {};
	}
	
	FString Value;
	Property.ExportText_InContainer(0, Value, this, this, nullptr, 0);
	return Value;
}

void UBPE_MetadataCollection::SetValueForProperty(const FProperty& Property, const FString& Value)
{
	if (const FBoolProperty* BoolProp = CastField<FBoolProperty>(&Property))
	{
		BoolProp->SetPropertyValue_InContainer(this, true);
	}
	else
	{
		Property.ImportText_InContainer(*Value, this, this, 0);
	}
}

void UBPE_MetadataCollection::InitializeValueForProperty(const FProperty& Property)
{
	for (int32 Idx = 0; Idx < Property.ArrayDim; Idx++)
	{
		uint8* DataPtr = Property.ContainerPtrToValuePtr<uint8>(this, Idx);
		const uint8* DefaultValue = Property.ContainerPtrToValuePtrForDefaults<uint8>(GetClass(), GetArchetype(), Idx);
		Property.CopyCompleteValue(DataPtr, DefaultValue);
	}
}

void UBPE_MetadataCollection::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UObject::PostEditChangeProperty(PropertyChangedEvent);

	if (!PropertyChangedEvent.Property)
	{
		return;
	}
	
	const FName PropertyName = PropertyChangedEvent.GetPropertyName();
	if (const auto OptionalValue = GetValueForProperty(*PropertyChangedEvent.Property))
	{
		CurrentWrapper.SetMetadata(PropertyName, *OptionalValue);
	}
	else
	{
		CurrentWrapper.RemoveMetadata(PropertyName);
	}
}

bool UBPE_MetadataCollection::IsPropertyVisible(const FProperty& Property) const
{
	return !Property.HasAnyPropertyFlags(CPF_DisableEditOnInstance);
}

bool UBPE_MetadataCollectionStruct::IsRelevantForContainedProperty(const FProperty& InProperty) const
{
	if (const FStructProperty* AsStruct = CastField<FStructProperty>(&InProperty))
	{
		return Structs.Contains(AsStruct->Struct);
	}
	return false;
}
