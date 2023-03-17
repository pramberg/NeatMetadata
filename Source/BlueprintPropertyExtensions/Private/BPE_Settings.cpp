// Copyright Viktor Pramberg. All Rights Reserved.


#include "BPE_Settings.h"
#include "BPE_CollectionTypes.h"

UBPE_Settings::UBPE_Settings()
{
	CategoryName = "Plugins";
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
