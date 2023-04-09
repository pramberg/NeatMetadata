// Copyright Viktor Pramberg. All Rights Reserved.


#include "BPE_Settings.h"
#include "BPE_CollectionTypes.h"

UBPE_Settings::UBPE_Settings()
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

void UBPE_Settings::ForEachCollection(TFunctionRef<FForEachCollectionSignature> Functor) const
{
	for (TObjectPtr<UBPE_MetadataCollection> Collection : MetadataCollectionInstances)
	{
		check(Collection);
		Functor(*Collection);
	}
}

void UBPE_Settings::RebuildMetadataCollections()
{
	MetadataCollectionInstances.Empty();
	
	if (AllowedCollections.IsEmpty())
	{
		// TODO: Support blueprint classes too? Right now they are not guaranteed to be loaded, so we have to load them through the asset registry.
		for (UClass* Class : TObjectRange<UClass>())
		{
			if (!Class->IsChildOf(UBPE_MetadataCollection::StaticClass()) || Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated))
			{
				continue;
			}

			if (DisallowedCollections.Contains(Class))
			{
				continue;
			}

			MetadataCollectionInstances.Add(NewObject<UBPE_MetadataCollection>(this, Class));
		}
	}
	else
	{
		MetadataCollectionInstances.Reserve(AllowedCollections.Num());

		for (TSoftClassPtr<UBPE_MetadataCollection> SoftClass : AllowedCollections)
		{
			if (SoftClass.IsNull())
			{
				continue;
			}

			const UClass* LoadedClass = SoftClass.LoadSynchronous();
			if (ensure(LoadedClass))
			{
				MetadataCollectionInstances.Add(NewObject<UBPE_MetadataCollection>(this, LoadedClass));
			}
		}
	}

	// Sort groups. Not sure if this is the best sort order, or if we should sort them differently.
	// General should maybe always be on top, for example?
	auto GetGroupName = [](const UBPE_MetadataCollection& InCollection)
	{
		const FString* GroupPtr = InCollection.GetClass()->FindMetaData(TEXT("Group"));
		return GroupPtr ? *GroupPtr : InCollection.GetClass()->GetName();
	};
	
	MetadataCollectionInstances.Sort([&](const UBPE_MetadataCollection& InA, const UBPE_MetadataCollection& InB)
	{
		return GetGroupName(InA) < GetGroupName(InB);
	});
}

void UBPE_Settings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, AllowedCollections)
		|| PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, DisallowedCollections))
	{
		RebuildMetadataCollections();
	}
}

void UBPE_Settings::PostInitProperties()
{
	Super::PostInitProperties();

	RebuildMetadataCollections();
}

UBPE_UserSettings::UBPE_UserSettings()
{
	CategoryName = "Plugins";
}
