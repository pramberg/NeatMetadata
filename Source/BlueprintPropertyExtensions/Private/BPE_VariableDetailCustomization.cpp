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
		FProperty* PropertyBeingCustomized = PropertyWrapper ? PropertyWrapper->GetProperty() : nullptr;
		if (!PropertyBeingCustomized)
			return;

		auto MetaWrapper = FBPE_MetadataWrapper{PropertyBeingCustomized, Blueprint};

		// The sorting seems to be as follows:
		// Variable: 4000
		// DefaultValue: 4001
		//
		// We want to insert our category in between, so push back Variable two steps, and our
		// Metadata category one step.
		{
			IDetailCategoryBuilder& VariableCategory = DetailLayout.EditCategory("Variable");
			VariableCategory.SetSortOrder(VariableCategory.GetSortOrder() - 2);
		}
		
		IDetailCategoryBuilder& MetadataCategory = DetailLayout.EditCategory("Metadata", LOCTEXT("MetadataCategoryTitle", "Metadata"));
		MetadataCategory.SetSortOrder(MetadataCategory.GetSortOrder() - 1);

		TMap<FName, IDetailGroup*> GroupNameToGroup;
		
		GetDefault<UBPE_Settings>()->ForEachCollection([&](UBPE_MetadataCollection& Collection)
		{
			if (!Collection.IsRelevantForProperty(*PropertyBeingCustomized))
			{
				return;
			}

			const UClass& CollectionClass = *Collection.GetClass();
			const bool bNoGroup = CollectionClass.HasMetaData(TEXT("NoGroup"));
			
			IDetailGroup* Group = nullptr;

			if (!bNoGroup)
			{
				const FString* GroupPtr = CollectionClass.FindMetaData(TEXT("Group"));
				const FName GroupName = GroupPtr ? FName(*GroupPtr) : CollectionClass.GetFName();
				if (IDetailGroup** FoundGroup = GroupNameToGroup.Find(GroupName))
				{
					Group = *FoundGroup;
				}

				if (!Group)
				{
					const FText GroupDisplayName = GroupPtr ? FText::FromString(*GroupPtr) : CollectionClass.GetDisplayNameText();
					Group = &MetadataCategory.AddGroup(GroupName, GroupDisplayName);
					
				   // Customize the header to allow tooltips on the group itself.
				   Group->HeaderRow()
				   [
					   SNew(SBox)
					   .ToolTipText(GroupPtr ? FText() : CollectionClass.GetToolTipText())
					   .VAlign(VAlign_Center)
					   [
						   SNew(STextBlock)
						   .Font(DetailLayout.GetDetailFont())
						   .Text(GroupDisplayName)
					   ]
				   ];

					GroupNameToGroup.Add(GroupName, Group);
				}
			}

			Collection.InitializeFromMetadata(MetaWrapper);

			Collection.ForEachVisibleProperty([&](const FProperty& Property)
			{
				if (const TSharedPtr<IPropertyHandle> Handle = DetailLayout.AddObjectPropertyData({ &Collection }, Property.GetFName()))
				{
					const TSharedPtr<SWidget> ValueWidget = Collection.CreateValueWidgetForProperty(Handle.ToSharedRef());
					if (Group)
					{
						if (ValueWidget)
						{
							Group->AddWidgetRow().NameContent()
							[
								Handle->CreatePropertyNameWidget()
							]
							.ValueContent()
							[
								ValueWidget.ToSharedRef()
							];
						}
						else
						{
							Group->AddPropertyRow(Handle.ToSharedRef());
						}
					}
					else
					{
						if (ValueWidget)
						{
							MetadataCategory.AddCustomRow(Property.GetDisplayNameText()).NameContent()
							[
								Handle->CreatePropertyNameWidget()
							]
							.ValueContent()
							[
								ValueWidget.ToSharedRef()
							];
						}
						else
						{
							MetadataCategory.AddProperty(Handle);
						}
					}
				}
			});
		});
		
		if (!PropertyBeingCustomized->GetMetaDataMap())
			return;

		auto& Group = MetadataCategory.AddGroup("All Metadata", LOCTEXT("All Metadata", "All Metadata"));

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
					.OnTextCommitted_Lambda
					(
						[Key, MetaWrapper](const FText& Text, ETextCommit::Type)
						{
							MetaWrapper.SetMetadata(Key, Text.ToString());
						}
					)
				];
		}
	}
}

#undef LOCTEXT_NAMESPACE
