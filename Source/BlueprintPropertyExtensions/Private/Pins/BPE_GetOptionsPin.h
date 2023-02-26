// Copyright Viktor Pramberg. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "EdGraphUtilities.h"

struct FBPE_GetOptionsPinFactory final : FGraphPanelPinFactory
{
	virtual TSharedPtr<SGraphPin> CreatePin(UEdGraphPin* Pin) const override;
};