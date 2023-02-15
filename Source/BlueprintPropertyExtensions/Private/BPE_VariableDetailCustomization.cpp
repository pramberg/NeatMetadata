// Copyright Viktor Pramberg. All Rights Reserved.


#include "BPE_VariableDetailCustomization.h"
#include "DetailLayoutBuilder.h"
#include "BlueprintEditorModule.h"
#include "BPE_MetadataCollection.h"
#include "BPE_Settings.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsEditorModule.h"
#include "IDetailGroup.h"
#include "Widgets/Input/SEditableText.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "BPE_MetadataWrapper.h"

#define LOCTEXT_NAMESPACE "BPE_VariableDetailCustomization"

TSharedPtr<IDetailCustomization> BPE_VariableDetailCustomization::MakeInstance(TSharedPtr<IBlueprintEditor> InBlueprintEditor)
{
	const TArray<UObject*>* Objects = (InBlueprintEditor.IsValid() ? InBlueprintEditor->GetObjectsCurrentlyBeingEdited() : nullptr);
	if (Objects)
	{
		TOptional<UBlueprint*> FinalBlueprint;
		for (UObject* Object : *Objects)
		{
			UBlueprint* Blueprint = Cast<UBlueprint>(Object);
			if (Blueprint == nullptr)
			{
				return nullptr;
			}
			if (FinalBlueprint.IsSet() && FinalBlueprint.GetValue() != Blueprint)
			{
				return nullptr;
			}
			FinalBlueprint = Blueprint;
		}

		if (FinalBlueprint.IsSet())
		{
			return MakeShared<BPE_VariableDetailCustomization>(FinalBlueprint.GetValue());
		}
	}

	return nullptr;
}

BPE_VariableDetailCustomization::BPE_VariableDetailCustomization(UBlueprint* InBlueprint) : Blueprint(InBlueprint)
{
}

void BPE_VariableDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailLayout.GetObjectsBeingCustomized(ObjectsBeingCustomized);
	if (ObjectsBeingCustomized.Num() > 0)
	{
		UPropertyWrapper* PropertyWrapper = Cast<UPropertyWrapper>(ObjectsBeingCustomized[0].Get());
		TWeakFieldPtr<FProperty> PropertyBeingCustomized = PropertyWrapper ? PropertyWrapper->GetProperty() : nullptr;
		if (!PropertyBeingCustomized.IsValid())
			return;

		auto MetaWrapper = FBPE_MetadataWrapper{PropertyBeingCustomized, Blueprint};

		for (auto Collection : GetDefault<UBPE_Settings>()->MetadataCollections)
		{
			if (Collection.IsNull())
			{
				continue;
			}
				
			auto* Obj = Collection.LoadSynchronous()->GetDefaultObject<UBPE_MetadataCollection>();
			if (Obj->IsRelevantForProperty(PropertyBeingCustomized.Get()))
			{
				auto& Category = DetailLayout.EditCategory("Variable");
				const FText Grp = Obj->GetGroup().Get(Obj->GetClass()->GetDisplayNameText());
				IDetailGroup& Group = Category.AddGroup(FName(Grp.ToString()), Grp);
				
				for (auto* Prop : TFieldRange<FProperty>(Obj->GetClass()))
				{
					if (!Obj->IsPropertyVisible(Prop->GetFName()))
						continue;
					
					if (const auto Handle = DetailLayout.AddObjectPropertyData({Obj}, Prop->GetFName()))
					{
						using FDelegateType = TDelegate<void(const FPropertyChangedEvent&)>;
						Handle->SetOnPropertyValueChangedWithData(FDelegateType::CreateLambda([Obj, MetaWrapper](const FPropertyChangedEvent& Event)
						{
							Obj->OnPropertyChanged(Event, MetaWrapper);
						}));
						Group.AddPropertyRow(Handle.ToSharedRef());
					}

					Obj->InitializeFromMetadata(MetaWrapper);
				}
				// const auto Handle = DetailLayout.AddObjectPropertyData({Obj}, FName("Categories"));
				// if (Handle)
				// {
				// 	auto& Category = DetailLayout.EditCategory("Variable");
				// 	Category.AddProperty(Handle);
				// }
				//
				// auto& Category = DetailLayout.EditCategory("Variable");
				// if (TOptional<FText> Grp = Obj->GetGroup())
				// {
				// 	Category.AddGroup(FName(Grp->ToString()), *Grp).AddWidgetRow()
				// 	[
				// 		Obj->CreateWidget(MetaWrapper)
				// 	];
				// }
				// else
				// {
				// 	Category.AddCustomRow(FText::FromString(Obj->GetName()))
				// 	[
				// 		Obj->CreateWidget(MetaWrapper)
				// 	];
				// }
			}
		}
		
		{
			auto& Group = DetailLayout
			   .EditCategory("Variable")
			   .AddGroup("EditCondition", LOCTEXT("EditCondition", "EditCondition"));
			
			Group.HeaderRow()
			[
				SNew(SOverlay)
				.ToolTipText(LOCTEXT("EditConditionGroupTooltip", "Metadata settings for conditionally disabling editing, or even hiding this property based on some condition."))
				+SOverlay::Slot()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Font(DetailLayout.GetDetailFont())
					.Text(LOCTEXT("EditConditionHeader", "Edit Condition"))
				]
			];

			static const FText EditConditionTooltip = LOCTEXT("EditConditionTooltip", "Condition used to enable editing this property. Note that the syntax is extensive, so you can do things like:\n"
				"`MyFloatVariable > 0.0`, which would only enable editing your property if the value is greater than zero.");
			Group.AddWidgetRow()
				.NameContent()
				[
					SNew(STextBlock)
					.ToolTipText(EditConditionTooltip)
					.Font(DetailLayout.GetDetailFont())
					.Text(LOCTEXT("EditConditionName", "Edit Condition"))
				]
				.ValueContent()
				[
					SNew(SEditableTextBox)
					.ToolTipText(EditConditionTooltip)
					.Text_Lambda([MetaWrapper]()
					{
						return FText::FromString(MetaWrapper.GetMetadata("EditCondition"));
					})
					.OnTextCommitted_Lambda([MetaWrapper](const FText& Text, ETextCommit::Type)
					{
						MetaWrapper.SetOrRemoveMetadata("EditCondition", Text.ToString());
					})
				];

		
			Group.AddWidgetRow()
				.NameContent()
				[
					SNew(STextBlock)
					.Font(DetailLayout.GetDetailFont())
					.Text(LOCTEXT("EditConditionHidesName", "Edit Condition Hides"))
				]
				.ValueContent()
				[
					SNew(SCheckBox)
					.IsChecked_Lambda([MetaWrapper]()
					{
						return MetaWrapper.HasMetadata("EditConditionHides") ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
					})
					.OnCheckStateChanged_Lambda([MetaWrapper](ECheckBoxState CheckState)
					{
						if (CheckState == ECheckBoxState::Checked)
						{
							MetaWrapper.SetMetadata("EditConditionHides", TEXT(""));
						}
						else
						{
							MetaWrapper.RemoveMetadata("EditConditionHides");
						}
					})
				];
		}

		if (const FStructProperty* StructProp = CastField<FStructProperty>(PropertyBeingCustomized.Get()))
		{
			if (StructProp->Struct == FGameplayTag::StaticStruct() || StructProp->Struct == FGameplayTagContainer::StaticStruct())
			{
				auto& Group = DetailLayout
					.EditCategory("Variable")
					.AddGroup("Gameplay Tag", LOCTEXT("Gameplay Tag Categories", "Categories (GameplayTag)"));

				Group.HeaderRow()
					[
						SNew(SOverlay)
						.ToolTipText(LOCTEXT("GameplayTagTooltip", "Set the Categories metadata that allows you to narrow down what GameplayTags are allowed to be selected."))
						+SOverlay::Slot()
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Font(DetailLayout.GetDetailFont())
							.Text(LOCTEXT("GameplayTag Categories", "GameplayTag Categories"))
						]
					];
				
				const TSharedPtr<FGameplayTag> TagRef = MakeShared<FGameplayTag>();
				TagRef->FromExportString(MetaWrapper.GetMetadata("Categories"));
				Group.AddWidgetRow()
				[
					IGameplayTagsEditorModule::Get()
					.MakeGameplayTagWidget(FOnSetGameplayTag::CreateLambda([MetaWrapper](const FGameplayTag& Tag)
					{
						MetaWrapper.SetMetadata("Categories", Tag.ToString());
					}), TagRef)
				];
			}
		}
		
		if (!PropertyBeingCustomized->GetMetaDataMap())
			return;

		auto& Group = DetailLayout
			.EditCategory("Variable")
			.AddGroup("All Metadata", LOCTEXT("All Metadata", "All Metadata"));
		
		for (auto& [Key, Value] : *PropertyBeingCustomized->GetMetaDataMap())
		{
			Group.AddWidgetRow()
				.NameContent()
				[
					SNew(STextBlock)
					.Font(DetailLayout.GetDetailFont())
					.Text(FText::FromName(Key))
				]
				.ValueContent()
				[
					SNew(SEditableTextBox)
					.Font(DetailLayout.GetDetailFont())
					.Text(FText::FromString(Value))
					.OnTextCommitted_Lambda([Key, MetaWrapper](const FText& Text, ETextCommit::Type)
					{
						MetaWrapper.SetMetadata(Key, Text.ToString());
					})
				];
		}
	}
}

#undef LOCTEXT_NAMESPACE
