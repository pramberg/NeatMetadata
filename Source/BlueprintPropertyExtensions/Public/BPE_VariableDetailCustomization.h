// Copyright Viktor Pramberg. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class IBlueprintEditor;

/**
 * 
 */
class BPE_VariableDetailCustomization : public IDetailCustomization
{
public:
	static TSharedPtr<IDetailCustomization> MakeInstance(TSharedPtr<IBlueprintEditor> InBlueprintEditor);
	BPE_VariableDetailCustomization(UBlueprint* InBlueprint);

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;
	
private:
	TWeakObjectPtr<UBlueprint> Blueprint;
};