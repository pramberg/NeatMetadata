// Copyright Viktor Pramberg. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"

// FBPE_FunctionSelectorItem is in the cpp file.
using FBPE_FunctionSelectorItemPtr = TSharedPtr<struct FBPE_FunctionSelectorItem>;
using SBPE_FunctionSelectorTreeView = STreeView<FBPE_FunctionSelectorItemPtr>;

DECLARE_DELEGATE_RetVal_TwoParams(bool, FBPE_FunctionSelectorFunctionFilter, const UFunction*, bool);
DECLARE_DELEGATE_RetVal(TOptional<FString>, FBPE_FunctionSelectorAddNewFunction);

// Widget that displays functions that match a filter.
//
// TODO: This class is basically only relevant for GetOptions right now.
// Should it be be more generic, or just be renamed to specify that it's for GetOptions?
class SBPE_FunctionSelector : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SBPE_FunctionSelector) : _MemberClass(nullptr) {}
	SLATE_ATTRIBUTE(UClass*, MemberClass)
	SLATE_EVENT(FBPE_FunctionSelectorFunctionFilter, FunctionFilter)
	SLATE_EVENT(FBPE_FunctionSelectorAddNewFunction, AddNewFunction)
	SLATE_END_ARGS()
	
	void Construct(const FArguments&, TSharedRef<IPropertyHandle> InPropertyHandle);

protected:
	FText GetText() const;
	TSharedRef<SWidget> OnGetMenuContent();
	void OnSelectionChanged(FBPE_FunctionSelectorItemPtr InSelection, ESelectInfo::Type InSelectInfo) const;
	void OnSearchTextChanged(const FText& ChangedText);
	void OnSearchTextCommitted(const FText& InText, ETextCommit::Type InCommitType);
	void SetCurrentItem(FBPE_FunctionSelectorItemPtr InItem) const;
	TSharedRef<class ITableRow> OnGenerateRow(FBPE_FunctionSelectorItemPtr InItem, const TSharedRef< class STableViewBase >& InParent) const;
	void RefreshFunctions();

private:
	TSharedPtr<IPropertyHandle> PropertyHandle;
	
	TArray<FBPE_FunctionSelectorItemPtr> Items;
	TArray<FBPE_FunctionSelectorItemPtr> FilteredItems;
	
	FText SearchText;
	TSharedPtr<SBPE_FunctionSelectorTreeView> TreeView;
	TSharedPtr<SComboButton> ComboButton;

	TAttribute<UClass*> MemberClass;
	FBPE_FunctionSelectorFunctionFilter FunctionFilter;
	FBPE_FunctionSelectorAddNewFunction AddNewFunction;
};
