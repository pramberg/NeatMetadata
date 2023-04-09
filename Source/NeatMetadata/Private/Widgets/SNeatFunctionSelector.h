// Copyright Viktor Pramberg. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"

// FNeatFunctionSelectorItem is in the cpp file.
using FNeatFunctionSelectorItemPtr = TSharedPtr<struct FNeatFunctionSelectorItem>;
using SNeatFunctionSelectorTreeView = STreeView<FNeatFunctionSelectorItemPtr>;

DECLARE_DELEGATE_RetVal_TwoParams(bool, FNeatFunctionSelectorFunctionFilter, const UFunction*, bool);
DECLARE_DELEGATE_RetVal(TOptional<FString>, FNeatFunctionSelectorAddNewFunction);

// Widget that displays functions that match a filter.
//
// TODO: This class is basically only relevant for GetOptions right now.
// Should it be be more generic, or just be renamed to specify that it's for GetOptions?
class SNeatFunctionSelector : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SNeatFunctionSelector) : _MemberClass(nullptr) {}
	SLATE_ATTRIBUTE(UClass*, MemberClass)
	SLATE_EVENT(FNeatFunctionSelectorFunctionFilter, FunctionFilter)
	SLATE_EVENT(FNeatFunctionSelectorAddNewFunction, AddNewFunction)
	SLATE_END_ARGS()
	
	void Construct(const FArguments&, TSharedRef<IPropertyHandle> InPropertyHandle);

protected:
	FText GetText() const;
	TSharedRef<SWidget> OnGetMenuContent();
	void OnSelectionChanged(FNeatFunctionSelectorItemPtr InSelection, ESelectInfo::Type InSelectInfo) const;
	void OnSearchTextChanged(const FText& ChangedText);
	void OnSearchTextCommitted(const FText& InText, ETextCommit::Type InCommitType);
	void SetCurrentItem(FNeatFunctionSelectorItemPtr InItem) const;
	TSharedRef<class ITableRow> OnGenerateRow(FNeatFunctionSelectorItemPtr InItem, const TSharedRef< class STableViewBase >& InParent) const;
	void RefreshFunctions();

private:
	TSharedPtr<IPropertyHandle> PropertyHandle;
	
	TArray<FNeatFunctionSelectorItemPtr> Items;
	TArray<FNeatFunctionSelectorItemPtr> FilteredItems;
	
	FText SearchText;
	TSharedPtr<SNeatFunctionSelectorTreeView> TreeView;
	TSharedPtr<SComboButton> ComboButton;

	TAttribute<UClass*> MemberClass;
	FNeatFunctionSelectorFunctionFilter FunctionFilter;
	FNeatFunctionSelectorAddNewFunction AddNewFunction;
};
