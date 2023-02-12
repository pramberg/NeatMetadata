// "// Copyright Viktor Pramberg. All Rights Reserved."


#include "BPE_VariableDetailCustomization.h"
#include "DetailLayoutBuilder.h"
#include "BlueprintEditorModule.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsEditorModule.h"
#include "IDetailGroup.h"
#include "Widgets/Input/SEditableText.h"
#include "Widgets/Input/SEditableTextBox.h"

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

namespace
{
	class FMetadataWrapper
	{
	public:
		FMetadataWrapper(TWeakFieldPtr<FProperty> InProperty, TWeakObjectPtr<UBlueprint> InBlueprint)
			: Property(InProperty), Blueprint(InBlueprint)
		{
		}
		
		void SetMetadata(FName Key, const FString& Value) const
		{
			FProperty* Prop;
			FBPVariableDescription* Desc;
			if (GetMetadataContainers(Prop, Desc))
			{
				Prop->SetMetaData(Key, *Value);
				Desc->SetMetaData(Key,Value);
			}
		}

		void SetOrRemoveMetadata(FName Key, const FString& Value) const
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

		void RemoveMetadata(FName Key) const
		{
			FProperty* Prop;
			FBPVariableDescription* Desc;
			if (GetMetadataContainers(Prop, Desc))
			{
				Prop->RemoveMetaData(Key);
				Desc->RemoveMetaData(Key);
			}
		}

		FString GetMetadata(FName Key) const
		{
			FProperty* Prop;
			FBPVariableDescription* Desc;
			if (GetMetadataContainers(Prop, Desc))
			{
				return Desc->HasMetaData(Key) ? Desc->GetMetaData(Key) : FString();
			}
			return FString();
		}
		
		bool HasMetadata(FName Key) const
		{
			FProperty* Prop;
			FBPVariableDescription* Desc;
			if (GetMetadataContainers(Prop, Desc))
			{
				return Desc->HasMetaData(Key);
			}
			return false;
		}
		
	private:
		bool GetMetadataContainers(FProperty*& OutProp, FBPVariableDescription*& OutDesc) const
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
		
		TWeakFieldPtr<FProperty> Property;
		TWeakObjectPtr<UBlueprint> Blueprint;
	};
	
	void OnSetGameplayTagContainer(const FGameplayTag& Tag, FMetadataWrapper MetadataWrapper)
	{
		MetadataWrapper.SetMetadata("Categories", Tag.ToString());
	}
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

		auto MetaWrapper = FMetadataWrapper{PropertyBeingCustomized, Blueprint};
		
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
				TagRef->FromExportString(StructProp->GetMetaData("Categories"));
				Group.AddWidgetRow()
				[
					IGameplayTagsEditorModule::Get()
					.MakeGameplayTagWidget(FOnSetGameplayTag::CreateStatic(&OnSetGameplayTagContainer, MetaWrapper), TagRef)
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
