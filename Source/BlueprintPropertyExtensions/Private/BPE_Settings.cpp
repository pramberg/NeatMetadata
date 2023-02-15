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
