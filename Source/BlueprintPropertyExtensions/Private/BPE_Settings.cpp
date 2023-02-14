// Copyright Viktor Pramberg. All Rights Reserved.


#include "BPE_Settings.h"
#include "BPE_MetadataCollection.h"

UBPE_Settings::UBPE_Settings()
{
	MetadataCollections.Add(UBPE_MetadataCollection_GameplayTagCategories::StaticClass());
	MetadataCollections.Add(UBPE_MetadataCollection_EditCondition::StaticClass());
	MetadataCollections.Add(UBPE_MetadataCollection_Units::StaticClass());
}
