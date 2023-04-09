// Copyright Viktor Pramberg. All Rights Reserved.
#pragma once
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "UObject/Class.h"
#include "Internationalization/Text.h"

class IPropertyHandle;
class SComboButton;

// Widget that displays all available interfaces.
class SNeatInterfaceSelector : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SNeatInterfaceSelector) {}
	SLATE_END_ARGS()

	void Construct(const FArguments&, TSharedRef<IPropertyHandle> InPropertyHandle);

protected:
	TSharedRef<SWidget> GetMenuContent();
	void OnClassPicked(UClass* InClass) const;
	UClass* GetClass() const;
	FText GetButtonText() const;

private:
	TSharedPtr<IPropertyHandle> PropertyHandle;
	TSharedPtr<SComboButton> ComboButton;
};