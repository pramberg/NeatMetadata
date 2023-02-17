// Copyright Viktor Pramberg. All Rights Reserved.


#include "BPE_Settings.h"
#include "BPE_CollectionTypes.h"

UBPE_Settings::UBPE_Settings()
{
	MetadataCollections = {
		UBPE_MetadataCollection_GameplayTagCategories::StaticClass(),
		UBPE_MetadataCollection_EditCondition::StaticClass(),
		UBPE_MetadataCollection_Units::StaticClass(),
		UBPE_MetadataCollection_Curves::StaticClass(),
		UBPE_MetadataCollection_AssetBundles::StaticClass(),
	};
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
	MetadataCollectionInstances.Empty(MetadataCollections.Num());

	for (TSoftClassPtr<UBPE_MetadataCollection> SoftClass : MetadataCollections)
	{
		if (!ensure(!SoftClass.IsNull()))
		{
			continue;
		}

		const UClass* LoadedClass = SoftClass.LoadSynchronous();
		MetadataCollectionInstances.Add(NewObject<UBPE_MetadataCollection>(this, LoadedClass));
	}
}

void UBPE_Settings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, MetadataCollections))
	{
		RebuildMetadataCollections();
	}
}

void UBPE_Settings::PostInitProperties()
{
	Super::PostInitProperties();

	RebuildMetadataCollections();
}
