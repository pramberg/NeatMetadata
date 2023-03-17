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
	const TArray<UObject*>* Objects = InBlueprintEditor.IsValid() ? InBlueprintEditor->GetObjectsCurrentlyBeingEdited() : nullptr;
	if (!Objects)
	{
		return nullptr;
	}
		
	UBlueprint* FinalBlueprint = nullptr;
	for (UObject* Object : *Objects)
	{
		UBlueprint* Blueprint = Cast<UBlueprint>(Object);
		if (!Blueprint || (FinalBlueprint && FinalBlueprint != Blueprint))
		{
			return nullptr;
		}
		FinalBlueprint = Blueprint;
	}

	check(FinalBlueprint);
	return MakeShared<BPE_VariableDetailCustomization>(FinalBlueprint);
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
		
		DetailLayout.SortCategories([](const TMap<FName, IDetailCategoryBuilder*>& InAllCategoryMap)
		{
			IDetailCategoryBuilder* ValueCategory = InAllCategoryMap["DefaultValueCategory"];
			IDetailCategoryBuilder* MetadataCategory = InAllCategoryMap["Metadata"];
			
			const int32 ValueSortOrder = ValueCategory->GetSortOrder();
			const int32 MetadataSortOrder = MetadataCategory->GetSortOrder();

			ValueCategory->SetSortOrder(MetadataSortOrder);
			MetadataCategory->SetSortOrder(ValueSortOrder);
		});
		
		IDetailCategoryBuilder& MetadataCategory = DetailLayout.EditCategory("Metadata", LOCTEXT("MetadataCategoryTitle", "Metadata"));

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
					IDetailPropertyRow& CreatedRow = Group ? Group->AddPropertyRow(Handle.ToSharedRef()) : MetadataCategory.AddProperty(Handle);

					if (ValueWidget)
					{
						CreatedRow.CustomWidget()
						.NameContent()
						[
							Handle->CreatePropertyNameWidget()
						]
						.ValueContent()
						[
							ValueWidget.ToSharedRef()
						];
					}
				}
			});
		});
		
		if (!PropertyBeingCustomized->GetMetaDataMap() || !GetDefault<UBPE_UserSettings>()->bShowAllMetadataCategory)
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
