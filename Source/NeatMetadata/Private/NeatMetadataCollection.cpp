// Copyright Viktor Pramberg. All Rights Reserved.
#include "NeatMetadataCollection.h"
#include "NeatMetadataWrapper.h"

UNeatMetadataCollection::UNeatMetadataCollection()
{
}

bool UNeatMetadataCollection::IsRelevantForContainedProperty(const FProperty& InProperty) const
{
	return true;
}

void UNeatMetadataCollection::InitializeFromMetadata(const FNeatMetadataWrapper& MetadataWrapper)
{
	CurrentWrapper = MetadataWrapper;

	ForEachVisibleProperty([this](const FProperty& Property)
	{
		if (CurrentWrapper.HasMetadata(Property.GetFName()))
		{
			const FString Value = CurrentWrapper.GetMetadata(Property.GetFName());
			ImportValueForProperty(Property, Value);
		}
		else
		{
			InitializeValueForProperty(Property);
		}
	});
}

void UNeatMetadataCollection::ForEachVisibleProperty(TFunctionRef<FForEachVisiblePropertySignature> Functor) const
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

bool UNeatMetadataCollection::IsRelevantForProperty(const FProperty& InProperty) const
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

TSharedPtr<SWidget> UNeatMetadataCollection::CreateValueWidgetForProperty(const TSharedRef<IPropertyHandle>& InHandle)
{
	return nullptr;
}

namespace
{
	template<typename T> struct TPropertyToHelper { using Type = void; };
	template<> struct TPropertyToHelper<FArrayProperty> { using Type = FScriptArrayHelper_InContainer; };
	template<> struct TPropertyToHelper<FMapProperty> { using Type = FScriptMapHelper_InContainer; };
	template<> struct TPropertyToHelper<FSetProperty> { using Type = FScriptSetHelper_InContainer; };
	
	template<typename T>
	bool IsEmptyContainerOfType(const FProperty& Property, const UObject* Container)
	{
		if (const T* AsContainer = CastField<T>(&Property))
		{
			const typename TPropertyToHelper<T>::Type Helper(AsContainer, Container);
			return Helper.Num() == 0;
		}
		
		return false;
	}

	bool IsEmptyContainer(const FProperty& Property, const UObject* Container)
	{
		return IsEmptyContainerOfType<FArrayProperty>(Property, Container)
			|| IsEmptyContainerOfType<FMapProperty>(Property, Container)
			|| IsEmptyContainerOfType<FSetProperty>(Property, Container);
	}
}

TOptional<FString> UNeatMetadataCollection::ExportValueForProperty(FProperty& Property) const
{
	if (const FBoolProperty* AsBool = CastField<FBoolProperty>(&Property))
	{
		static const FString TrueReturnValue = FString(TEXT("true"));
		return AsBool->GetPropertyValue_InContainer(this) ? TOptional(TrueReturnValue) : NullOpt;
	}

	if (const FObjectPropertyBase* AsObject = CastField<FObjectPropertyBase>(&Property))
	{
		if (!AsObject->GetObjectPropertyValue_InContainer(this))
		{
			return {};
		}
	}
	
	if (IsEmptyContainer(Property, this))
	{
		return {};
	}

	FString Value;
	if (Property.ArrayDim == 1)
	{
		Property.ExportText_InContainer(0, Value, this, this, nullptr, 0);
	}
	else
	{
		const void* ValueAddr = Property.ContainerPtrToValuePtr<void>(this);
		FArrayProperty::ExportTextInnerItem(Value, &Property, ValueAddr, Property.ArrayDim, ValueAddr, Property.ArrayDim);
	}

	if (Property.IsA<FStrProperty>() && Value.IsEmpty())
	{
		return {};
	}
	
	return Value;
}

void UNeatMetadataCollection::ImportValueForProperty(const FProperty& Property, const FString& Value)
{
	if (const FBoolProperty* BoolProp = CastField<FBoolProperty>(&Property))
	{
		BoolProp->SetPropertyValue_InContainer(this, true);
	}
	else if (Property.ArrayDim == 1)
	{
		Property.ImportText_InContainer(*Value, this, this, 0);
	}
	else
	{
		void* ValueAddr = Property.ContainerPtrToValuePtr<void>(this);
		FArrayProperty::ImportTextInnerItem(*Value, &Property, ValueAddr, PPF_None, this);
	}
}

void UNeatMetadataCollection::InitializeValueForProperty(const FProperty& Property)
{
	for (int32 Idx = 0; Idx < Property.ArrayDim; Idx++)
	{
		uint8* DataPtr = Property.ContainerPtrToValuePtr<uint8>(this, Idx);
		const uint8* DefaultValue = Property.ContainerPtrToValuePtrForDefaults<uint8>(GetClass(), GetArchetype(), Idx);
		Property.CopyCompleteValue(DataPtr, DefaultValue);
	}
}

void UNeatMetadataCollection::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UObject::PostEditChangeProperty(PropertyChangedEvent);

	if (!PropertyChangedEvent.MemberProperty)
	{
		return;
	}
	
	const FName PropertyName = PropertyChangedEvent.GetMemberPropertyName();
	if (const auto OptionalValue = ExportValueForProperty(*PropertyChangedEvent.MemberProperty))
	{
		CurrentWrapper.SetMetadata(PropertyName, *OptionalValue);
	}
	else
	{
		CurrentWrapper.RemoveMetadata(PropertyName);
	}
}

bool UNeatMetadataCollection::IsPropertyVisible(const FProperty& Property) const
{
	return !Property.HasAnyPropertyFlags(CPF_DisableEditOnInstance);
}

bool UNeatMetadataCollectionStruct::IsRelevantForContainedProperty(const FProperty& InProperty) const
{
	if (const FStructProperty* AsStruct = CastField<FStructProperty>(&InProperty))
	{
		return Structs.Contains(AsStruct->Struct);
	}
	return false;
}
