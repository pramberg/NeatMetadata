// Copyright Viktor Pramberg. All Rights Reserved.


#include "BPE_MetadataCollection.h"

#include "BPE_MetadataWrapper.h"

UBPE_MetadataCollection::UBPE_MetadataCollection()
{
	PropertyClass = FProperty::StaticClass();
}

TSharedRef<SWidget> UBPE_MetadataCollection::CreateWidget(const FBPE_MetadataWrapper& MetadataWrapper)
{
	InitializeFromMetadata(MetadataWrapper);
	
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.bHideSelectionTip = true;
	DetailsViewArgs.bShowScrollBar = false;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.bShowOptions = false;
	DetailsViewArgs.bShowPropertyMatrixButton = false;
	//DetailsViewArgs.NotifyHook = this;
	auto DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	
	DetailsView->GetIsPropertyVisibleDelegate().BindWeakLambda(this, [this](const FPropertyAndParent& PropertyAndParent)
	{
		return IsPropertyVisible(PropertyAndParent.Property.GetFName());
	});

	DetailsView->GetIsCustomRowVisibleDelegate().BindWeakLambda(this, [this](FName InRowName, FName /*InParentName*/)
	{
		return IsPropertyVisible(InRowName);
	});

	DetailsView->OnFinishedChangingProperties().AddUObject(this, &ThisClass::OnPropertyChanged, MetadataWrapper);
	DetailsView->SetObject(this);
	
	return DetailsView;
}

bool UBPE_MetadataCollection::IsRelevantForProperty(FProperty* InProperty) const
{
	return InProperty->GetClass()->IsChildOf(PropertyClass);
}

TOptional<FText> UBPE_MetadataCollection::GetGroup() const
{
	return {};
}

void UBPE_MetadataCollection::InitializeFromMetadata(const FBPE_MetadataWrapper& MetadataWrapper)
{
	for (const auto* Property : TFieldRange<FProperty>(GetClass()))
	{
		if (Property->HasAnyPropertyFlags(CPF_DisableEditOnInstance))
			continue;

		if (MetadataWrapper.HasMetadata(Property->GetFName()))
		{
			if (const FBoolProperty* BoolProp = CastField<FBoolProperty>(Property))
			{
				BoolProp->SetPropertyValue_InContainer(this, true);
			}
			else
			{
				const FString Value = MetadataWrapper.GetMetadata(Property->GetFName());
				SetValueForProperty(*Property, Value);
			}
		}
		else
		{
			InitializeValueForProperty(*Property);
		}
	}
}

FString UBPE_MetadataCollection::GetValueForProperty(FProperty& Property) const
{
	FString Value;
	Property.ExportText_InContainer(0, Value, this, this, nullptr, 0);
	return Value;
}

void UBPE_MetadataCollection::SetValueForProperty(const FProperty& Property, const FString& Value)
{
	Property.ImportText_InContainer(*Value, this, this, 0);
}

void UBPE_MetadataCollection::InitializeValueForProperty(const FProperty& Property)
{
	Property.InitializeValue_InContainer(this);
}

void UBPE_MetadataCollection::OnPropertyChanged(const FPropertyChangedEvent& PropChanged, FBPE_MetadataWrapper MetadataWrapper)
{
	const FName PropertyName = PropChanged.Property->GetFName();
	if (const FBoolProperty* AsBool = CastField<FBoolProperty>(PropChanged.Property))
	{
		if (AsBool->GetPropertyValue_InContainer(this))
		{
			MetadataWrapper.SetMetadata(PropertyName, TEXT(""));
		}
		else
		{
			MetadataWrapper.RemoveMetadata(PropertyName);
		}
	}
	else
	{
		MetadataWrapper.SetMetadata(PropertyName, GetValueForProperty(*PropChanged.Property));
	}
}

bool UBPE_MetadataCollection::IsPropertyVisible(FName PropertyName)
{
	const auto* Prop = FindFProperty<FProperty>(GetClass(), PropertyName);
	return Prop ? !Prop->HasAnyPropertyFlags(CPF_DisableEditOnInstance) : true;
}

/*
 * *****************************
 * UBPE_MetadataCollectionStruct
 * *****************************
 */

bool UBPE_MetadataCollectionStruct::IsRelevantForProperty(FProperty* InProperty) const
{
	if (const FStructProperty* AsStruct = CastField<FStructProperty>(InProperty))
	{
		return Structs.Contains(AsStruct->Struct);
	}
	return false;
}
