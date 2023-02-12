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
	void OnSetGameplayTagContainer(const FGameplayTag& Tag, TWeakFieldPtr<FStructProperty> Prop, TWeakObjectPtr<UBlueprint> Blueprint)
	{
		if (!Prop.IsValid())
			return;

		Prop->SetMetaData("Categories", Tag.ToString());

		if (!Blueprint.IsValid() || !Prop.IsValid())
			return;

		UBlueprint* BP = Blueprint.Get();
		FProperty* Property = Prop.Get();

		FBPVariableDescription* Desc = BP->NewVariables.FindByPredicate([Property](const FBPVariableDescription& InDesc) { return InDesc.VarName == Property->GetFName(); });
		check(Desc);

		Desc->SetMetaData("Categories",Tag.ToString());
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

		if (FStructProperty* StructProp = CastField<FStructProperty>(PropertyBeingCustomized.Get()))
		{
			if (StructProp->Struct == FGameplayTag::StaticStruct() || StructProp->Struct == FGameplayTagContainer::StaticStruct())
			{
				auto& Group = DetailLayout.EditCategory("Variable")
				.AddGroup("Gameplay Tag", LOCTEXT("GameplayTag Categories", "Categories (GameplayTag)"));

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

				auto WeakProp = TWeakFieldPtr<FStructProperty>(StructProp);
				const TSharedPtr<FGameplayTag> TagRef = MakeShared<FGameplayTag>();
				TagRef->FromExportString(StructProp->GetMetaData("Categories"));
				Group.AddWidgetRow()
				[
					IGameplayTagsEditorModule::Get()
					.MakeGameplayTagWidget(FOnSetGameplayTag::CreateStatic(&OnSetGameplayTagContainer, WeakProp, Blueprint), TagRef)
				];
			}
		}

		if (!PropertyBeingCustomized->GetMetaDataMap())
			return;

		auto& Group = DetailLayout
			.EditCategory("Variable")
			.AddGroup("Test", LOCTEXT("All Metadata", "All Metadata"));
		
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
					.OnTextCommitted_Lambda([Key, PropertyBeingCustomized](const FText& Text, ETextCommit::Type)
					{
						if (PropertyBeingCustomized.IsValid())
						{
							PropertyBeingCustomized.Get()->SetMetaData(Key, *Text.ToString());
						}
					})
				];
		}
	}
}

#undef LOCTEXT_NAMESPACE
