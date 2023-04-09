// Copyright Viktor Pramberg. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class IBlueprintEditor;

/**
 * 
 */
class FNeatMetadataDetailCustomization : public IDetailCustomization
{
public:
	static TSharedPtr<IDetailCustomization> MakeInstance(TSharedPtr<IBlueprintEditor> InBlueprintEditor);
	FNeatMetadataDetailCustomization(UBlueprint* InBlueprint);

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;
	
private:
	TWeakObjectPtr<UBlueprint> Blueprint;
};