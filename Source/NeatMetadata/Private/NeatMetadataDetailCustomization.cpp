// Copyright Viktor Pramberg. All Rights Reserved.


#include "NeatMetadataDetailCustomization.h"
#include "DetailLayoutBuilder.h"
#include "BlueprintEditorModule.h"
#include "NeatMetadataCollection.h"
#include "NeatMetadataSettings.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailGroup.h"
#include "Widgets/Input/SEditableText.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "NeatMetadataWrapper.h"

#define LOCTEXT_NAMESPACE "NeatMetadataDetailCustomization"

TSharedPtr<IDetailCustomization> FNeatMetadataDetailCustomization::MakeInstance(TSharedPtr<IBlueprintEditor> InBlueprintEditor)
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
	return MakeShared<FNeatMetadataDetailCustomization>(FinalBlueprint);
}

FNeatMetadataDetailCustomization::FNeatMetadataDetailCustomization(UBlueprint* InBlueprint) : Blueprint(InBlueprint)
{
}

void FNeatMetadataDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailLayout.GetObjectsBeingCustomized(ObjectsBeingCustomized);
	if (ObjectsBeingCustomized.Num() > 0)
	{
		UPropertyWrapper* PropertyWrapper = Cast<UPropertyWrapper>(ObjectsBeingCustomized[0].Get());
		FProperty* PropertyBeingCustomized = PropertyWrapper ? PropertyWrapper->GetProperty() : nullptr;
		if (!PropertyBeingCustomized)
		{
			return;
		}

		FNeatMetadataWrapper MetaWrapper { PropertyBeingCustomized, Blueprint };
		if (!MetaWrapper.IsValid())
		{
			return;
		}
		
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
		
		GetDefault<UNeatMetadataSettings>()->ForEachCollection([&](UNeatMetadataCollection& Collection)
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

					FText Tooltip = GroupPtr ? FText() : CollectionClass.GetToolTipText();
					if (GroupPtr)
					{
						if (const FText* FoundTooltip = GetDefault<UNeatMetadataSettings>()->GroupTooltips.Find(GroupName))
						{
							Tooltip = *FoundTooltip;
						}
					}
					
					// Customize the header to allow tooltips on the group itself.
					Group->HeaderRow()
					.NameContent()
					[
						SNew(STextBlock)
						.ToolTipText(Tooltip)
						.Font(DetailLayout.GetDetailFont())
						.Text(GroupDisplayName)
					];

					GroupNameToGroup.Add(GroupName, Group);
				}
			}

			Collection.InitializeFromMetadata(MetaWrapper);

			Collection.ForEachVisibleProperty([&](const FProperty& Property)
			{
				if (const TSharedPtr<IPropertyHandle> Handle = DetailLayout.AddObjectPropertyData({ &Collection }, Property.GetFName()))
				{
					IDetailPropertyRow& CreatedRow = Group ? Group->AddPropertyRow(Handle.ToSharedRef()) : MetadataCategory.AddProperty(Handle);
					if (const TSharedPtr<SWidget> ValueWidget = Collection.CreateValueWidgetForProperty(Handle.ToSharedRef()))
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
		
		if (!PropertyBeingCustomized->GetMetaDataMap() || !GetDefault<UNeatMetadataUserSettings>()->bShowAllMetadataCategory)
			return;

		IDetailGroup& Group = MetadataCategory.AddGroup("All Metadata", LOCTEXT("All Metadata", "All Metadata"));
		Group.HeaderRow()
		.NameContent()
		[
			SNew(STextBlock)
			.ToolTipText(LOCTEXT("All Metadata Tooltip", "Displays all metadata on this property and provides some actions that can be taken on them."))
			.Font(DetailLayout.GetDetailFont())
			.Text(LOCTEXT("All Metadata", "All Metadata"))
		]
		.ExtensionContent()
		[
			SNew(SButton)
			.ToolTipText(LOCTEXT("RemoveAllMetadataTooltip", "Remove all metadata."))
			.ButtonStyle(FAppStyle::Get(), "SimpleButton")
			.OnClicked_Lambda([MetaWrapper]()
			{
				const FText Desc = FText::Format(
					INVTEXT("Removing all metadata from property [{0}]"),
					MetaWrapper.GetProperty()->GetDisplayNameText()
				);
				FScopedTransaction Transaction(Desc);
				
				TArray<FName> MetadataNames;
				MetaWrapper.GetProperty()->GetMetaDataMap()->GenerateKeyArray(MetadataNames);
				for (const FName& Name : MetadataNames)
				{
					MetaWrapper.RemoveMetadata(Name);
				}
				return FReply::Handled();
			})
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush(TEXT("Icons.X")))
			]
		];

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
							const FText Desc = FText::Format(
								INVTEXT("Setting metadata on property [{0}] to [{1}: {2}]"),
								MetaWrapper.GetProperty()->GetDisplayNameText(),
								FText::FromName(Key),
								Text
							);
							const FScopedTransaction Transaction(Desc);
							MetaWrapper.SetMetadata(Key, Text.ToString());
						}
					)
				]
				.ExtensionContent()
				[
					SNew(SButton)
					.ToolTipText(LOCTEXT("RemoveMetadataTooltip", "Remove metadata."))
					.ButtonStyle(FAppStyle::Get(), "SimpleButton")
					.OnClicked_Lambda([Key, MetaWrapper]()
					{
						const FText Desc = FText::Format(
							INVTEXT("Removing metadata with key [{0}] from property [{1}]"),
							FText::FromName(Key),
							MetaWrapper.GetProperty()->GetDisplayNameText()
						);
						const FScopedTransaction Transaction(Desc);
						
						MetaWrapper.RemoveMetadata(Key);
						return FReply::Handled();
					})
					[
						SNew(SImage)
						.Image(FAppStyle::GetBrush(TEXT("Icons.X")))
					]
				];
		}
	}
}

#undef LOCTEXT_NAMESPACE
