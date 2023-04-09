// Copyright Viktor Pramberg. All Rights Reserved.


#include "NeatMetadataSettings.h"
#include "NeatMetadataCollection.h"

UNeatMetadataSettings::UNeatMetadataSettings()
{
	CategoryName = "Plugins";

#define LOCTEXT_NAMESPACE "Blueprint Property Extensions"
	GroupTooltips = {
		{ "General", LOCTEXT("GeneralTooltip", "Common metadata.") },
		{ "Array", LOCTEXT("ArrayTooltip", "Metadata related to arrays.") },
		{ "Map", LOCTEXT("MapTooltip", "Metadata related to maps.") },
		{ "Text", LOCTEXT("TextTooltip", "Metadata related to properties that are edited using a text box.") },
		{ "Paths", LOCTEXT("PathsTooltip", "Metadata related to directory paths and file paths.") },
	};
#undef LOCTEXT_NAMESPACE
}

void UNeatMetadataSettings::ForEachCollection(TFunctionRef<FForEachCollectionSignature> Functor) const
{
	for (TObjectPtr<UNeatMetadataCollection> Collection : MetadataCollectionInstances)
	{
		check(Collection);
		Functor(*Collection);
	}
}

void UNeatMetadataSettings::RebuildMetadataCollections()
{
	MetadataCollectionInstances.Empty();
	
	if (AllowedCollections.IsEmpty())
	{
		// TODO: Support blueprint classes too? Right now they are not guaranteed to be loaded, so we have to load them through the asset registry.
		for (UClass* Class : TObjectRange<UClass>())
		{
			if (!Class->IsChildOf(UNeatMetadataCollection::StaticClass()) || Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated))
			{
				continue;
			}

			if (DisallowedCollections.Contains(Class))
			{
				continue;
			}

			MetadataCollectionInstances.Add(NewObject<UNeatMetadataCollection>(this, Class));
		}
	}
	else
	{
		MetadataCollectionInstances.Reserve(AllowedCollections.Num());

		for (TSoftClassPtr<UNeatMetadataCollection> SoftClass : AllowedCollections)
		{
			if (SoftClass.IsNull())
			{
				continue;
			}

			const UClass* LoadedClass = SoftClass.LoadSynchronous();
			if (ensure(LoadedClass))
			{
				MetadataCollectionInstances.Add(NewObject<UNeatMetadataCollection>(this, LoadedClass));
			}
		}
	}

	// Sort groups. Not sure if this is the best sort order, or if we should sort them differently.
	// General should maybe always be on top, for example?
	auto GetGroupName = [](const UNeatMetadataCollection& InCollection)
	{
		const FString* GroupPtr = InCollection.GetClass()->FindMetaData(TEXT("Group"));
		return GroupPtr ? *GroupPtr : InCollection.GetClass()->GetName();
	};
	
	MetadataCollectionInstances.Sort([&](const UNeatMetadataCollection& InA, const UNeatMetadataCollection& InB)
	{
		return GetGroupName(InA) < GetGroupName(InB);
	});
}

void UNeatMetadataSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, AllowedCollections)
		|| PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, DisallowedCollections))
	{
		RebuildMetadataCollections();
	}
}

void UNeatMetadataSettings::PostInitProperties()
{
	Super::PostInitProperties();

	RebuildMetadataCollections();
}

UNeatMetadataUserSettings::UNeatMetadataUserSettings()
{
	CategoryName = "Plugins";
}
