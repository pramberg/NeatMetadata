// Copyright Viktor Pramberg. All Rights Reserved.
#include "BPE_GetOptionsPin.h"
#include "K2Node_CallFunction.h"
#include "PropertyCustomizationHelpers.h"
#include "PropertyPathHelpers.h"
#include "SGraphPin.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class SBPE_GetOptionsPin final : public SGraphPin
{
public:
	SLATE_BEGIN_ARGS(SBPE_GetOptionsPin) {}
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj, TArray<TSharedPtr<FString>> InOptions);

private:
	void OnGetStrings(TArray<TSharedPtr<FString>>& OutOptions, TArray<TSharedPtr<SToolTip>>& OutTooltips, TArray<bool>& OutRestrictedItems) const;
	FString OnGetValue() const;
	void OnValueSelected(const FString& InValue) const;
	virtual TSharedRef<SWidget>	GetDefaultValueWidget() override;
	
	TArray<TSharedPtr<FString>> Options;
	TSharedPtr<SBox> OptionsContainer;
};

void SBPE_GetOptionsPin::Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj, TArray<TSharedPtr<FString>> InOptions)
{
	Options = InOptions;
		
	SGraphPin::Construct(SGraphPin::FArguments(), InGraphPinObj);
}

void SBPE_GetOptionsPin::OnGetStrings(TArray<TSharedPtr<FString>>& OutOptions, TArray<TSharedPtr<SToolTip>>& OutTooltips, TArray<bool>& OutRestrictedItems) const
{
	OutOptions = Options;
}

FString SBPE_GetOptionsPin::OnGetValue() const
{
	return GraphPinObj ? GraphPinObj->GetDefaultAsString() : FString();
}

void SBPE_GetOptionsPin::OnValueSelected(const FString& InValue) const
{
	if (GraphPinObj->IsPendingKill())
	{
		return;
	}
			
	if (GraphPinObj->GetDefaultAsString() != InValue)
	{
		const FScopedTransaction Transaction(NSLOCTEXT("SBPE_GetOptionsPin", "OnValueSelected", "Change String Pin Value"));
		GraphPinObj->Modify();
		GraphPinObj->GetSchema()->TrySetDefaultValue(*GraphPinObj, InValue);
	}
}

TSharedRef<SWidget> SBPE_GetOptionsPin::GetDefaultValueWidget()
{
	FPropertyComboBoxArgs Args;
	Args.OnGetStrings.BindSP(this, &SBPE_GetOptionsPin::OnGetStrings);
	Args.OnGetValue.BindSP(this, &SBPE_GetOptionsPin::OnGetValue);
	Args.OnValueSelected.BindSP(this, &SBPE_GetOptionsPin::OnValueSelected);

	const FString CurrentValue = OnGetValue();
	if (CurrentValue.IsEmpty() || !Options.ContainsByPredicate([&](const auto& SharedStr) {return *SharedStr == CurrentValue; }))
	{
		// TODO: Is this good? Should we produce a warning?
		// Producing a warning at this point doesn't seem straight forward.
		// This solution results in a double modify of the blueprint, so you have to compile twice to make it go green...
		OnValueSelected(*Options[0]);
	}
	
	return SAssignNew(OptionsContainer, SBox)
		.Visibility(this, &SGraphPin::GetDefaultValueVisibility)
		[
			PropertyCustomizationHelpers::MakePropertyComboBox(Args)
		];
}



TSharedPtr<SGraphPin> FBPE_GetOptionsPinFactory::CreatePin(UEdGraphPin* Pin) const
{
	if (Pin->Direction == EGPD_Output)
	{
		return nullptr;
	}
	
	const FName Category = Pin->PinType.PinCategory;
	if (Category != UEdGraphSchema_K2::PC_String && Category != UEdGraphSchema_K2::PC_Name)
	{
		return nullptr;
	}

	static const FName GetOptionsKey("GetOptions");
	FString GetOptionsFunctionName = Pin->GetOwningNode()->GetPinMetaData(Pin->GetFName(), GetOptionsKey);
	if (GetOptionsFunctionName.IsEmpty())
	{
		return nullptr;
	}

	// All of this is based on FPropertyHandleBase::GeneratePossibleValues(), but simplified.
	const UK2Node_CallFunction* CallFunction = Cast<UK2Node_CallFunction>(Pin->GetOwningNode());
	// Assume the function we will retrieve options from is in the same class as the function we're calling.
	UObject* FunctionOuter = CallFunction->GetTargetFunction()->GetOuterUClass()->GetDefaultObject();

	if (GetOptionsFunctionName.Contains(TEXT(".")))
	{
		const UFunction* GetOptionsFunction = FindObject<UFunction>(nullptr, *GetOptionsFunctionName, true);

		if (ensureMsgf(GetOptionsFunction && GetOptionsFunction->HasAnyFunctionFlags(EFunctionFlags::FUNC_Static), TEXT("Invalid GetOptions: %s"), *GetOptionsFunctionName))
		{
			FunctionOuter = GetOptionsFunction->GetOuterUClass()->GetDefaultObject();
			GetOptionsFunction->GetName(GetOptionsFunctionName);
		}
	}
		
	if (!FunctionOuter)
	{
		return nullptr;
	}

	TArray<FString> Options;
	{
		const FEditorScriptExecutionGuard ScriptExecutionGuard;

		const FCachedPropertyPath Path(GetOptionsFunctionName);
		if (!PropertyPathHelpers::GetPropertyValue(FunctionOuter, Path, Options))
		{
			TArray<FName> NameOptions;
			if (PropertyPathHelpers::GetPropertyValue(FunctionOuter, Path, NameOptions))
			{
				Algo::Transform(NameOptions, Options, [](const FName& InName) { return InName.ToString(); });
			}
		}
	}
	
	if (Options.Num() == 0)
	{
		return nullptr;
	}

	TArray<TSharedPtr<FString>> SharedOptions;
	Algo::Transform(Options, SharedOptions, [](const FString& InString) { return MakeShared<FString>(InString); });

	return SNew(SBPE_GetOptionsPin, Pin, SharedOptions);
}