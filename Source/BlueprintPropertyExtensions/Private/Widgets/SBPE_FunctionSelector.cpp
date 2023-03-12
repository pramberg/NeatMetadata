// Copyright Viktor Pramberg. All Rights Reserved.
#include "SBPE_FunctionSelector.h"
#include "Widgets/Input/SSearchBox.h"
#include "SListViewSelectorDropdownMenu.h"

enum class EBPE_FunctionSelectorItemType : uint8
{
	None,
	Class,
	MemberFunction,
	StaticFunction,
	AddNewFunction,
};

struct FBPE_FunctionSelectorItem
{
	explicit FBPE_FunctionSelectorItem(const UClass* InClass) :
		Object(InClass),
		ItemType(EBPE_FunctionSelectorItemType::Class),
		ToolTip(InClass ? InClass->GetToolTipText() : FText())
	{}
	
	explicit FBPE_FunctionSelectorItem(const UFunction* InFunction, const bool bIsMemberFunction) :
		Object(InFunction),
		ItemType(bIsMemberFunction ? EBPE_FunctionSelectorItemType::MemberFunction : EBPE_FunctionSelectorItemType::StaticFunction),
		ToolTip(InFunction ? InFunction->GetToolTipText() : FText())
	{}

	explicit FBPE_FunctionSelectorItem(const FText InText, const FText InToolTip = FText(), EBPE_FunctionSelectorItemType InItemType = EBPE_FunctionSelectorItemType::None) :
		ItemType(InItemType),
		DisplayName(InText),
		ToolTip(InToolTip)
	{}

	void SetDisplayName(const FText InText)
	{
		DisplayName = InText;
	}

protected:
	TWeakObjectPtr<const UStruct> Object = nullptr;
	EBPE_FunctionSelectorItemType ItemType = EBPE_FunctionSelectorItemType::None;
	FText DisplayName;
	FText ToolTip;
	TArray<FBPE_FunctionSelectorItemPtr> Children;

	friend class SBPE_FunctionSelector;
};

void SBPE_FunctionSelector::Construct(const FArguments& InArgs, TSharedRef<IPropertyHandle> InPropertyHandle)
{
	PropertyHandle = InPropertyHandle;

	FunctionFilter = InArgs._FunctionFilter;
	AddNewFunction = InArgs._AddNewFunction;
	MemberClass = InArgs._MemberClass;
		
	ChildSlot
	[
		SAssignNew(ComboButton, SComboButton)
		.OnGetMenuContent(this, &SBPE_FunctionSelector::OnGetMenuContent)
		.ButtonContent()
		[
			SNew(STextBlock).Text(this, &SBPE_FunctionSelector::GetText)
		]
	];
}

FText SBPE_FunctionSelector::GetText() const
{
	FString Value;
	PropertyHandle->GetValue(Value);
	if (const int32 Index = Value.Find(TEXT(".")))
	{
		Value = Value.RightChop(Index + 1);
	}
	return FText::FromString(Value);
}

TSharedRef<SWidget> SBPE_FunctionSelector::OnGetMenuContent()
{
	RefreshFunctions();

	SearchText = FText();
	FilteredItems.Reset();
	FilteredItems.Append(Items);

	TSharedPtr<SSearchBox> SearchBox;
	SAssignNew(SearchBox, SSearchBox)
	.HintText(INVTEXT("Search"))
	.OnTextChanged(this, &SBPE_FunctionSelector::OnSearchTextChanged)
	.OnTextCommitted(this, &SBPE_FunctionSelector::OnSearchTextCommitted);
	
	SAssignNew(TreeView, SBPE_FunctionSelectorTreeView)
	.TreeItemsSource(&FilteredItems)
	.SelectionMode(ESelectionMode::Single)
	.ItemHeight(32)
	.OnSelectionChanged(this, &SBPE_FunctionSelector::OnSelectionChanged)
	.OnGetChildren_Lambda([this](FBPE_FunctionSelectorItemPtr InParent, TArray<FBPE_FunctionSelectorItemPtr>& OutChildren) { OutChildren = InParent->Children; })
	.OnGenerateRow(this, &SBPE_FunctionSelector::OnGenerateRow);

	ComboButton->SetMenuContentWidgetToFocus(SearchBox);
	if (!FilteredItems.IsEmpty())
	{
		TreeView->SetItemExpansion(FilteredItems[0], true);
	}
	
	return SNew(SListViewSelectorDropdownMenu<FBPE_FunctionSelectorItemPtr>, SearchBox, TreeView)
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.0f)
		[
			SearchBox.ToSharedRef()
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.0f)
		[
			SNew(SBox)
			.HeightOverride(400.0f)
			.WidthOverride(300.0f)
			[
				TreeView.ToSharedRef()
			]
		]
	];
}

void SBPE_FunctionSelector::OnSelectionChanged(FBPE_FunctionSelectorItemPtr InSelection, ESelectInfo::Type InSelectInfo) const
{
	if (!InSelection.IsValid())
	{
		return;
	}
		
	if (InSelection->Children.Num())
	{
		const bool bIsExpanded = TreeView->IsItemExpanded(InSelection);
		TreeView->SetItemExpansion(InSelection, !bIsExpanded);
			
		if (InSelectInfo == ESelectInfo::OnMouseClick)
		{
			TreeView->ClearSelection();
		}
	}
	else
	{
		SetCurrentItem(InSelection);
	}
}

void SBPE_FunctionSelector::OnSearchTextChanged(const FText& ChangedText)
{
	SearchText = ChangedText;

	FilteredItems.Reset();

	if (SearchText.IsEmpty())
	{
		FilteredItems.Append(Items);
		
		if (!FilteredItems.IsEmpty())
		{
			TreeView->SetItemExpansion(FilteredItems[0], true);
		}
	}
	else
	{
		for (const FBPE_FunctionSelectorItemPtr& Item : Items)
		{
			FBPE_FunctionSelectorItemPtr FilteredItem = nullptr;
			for (const FBPE_FunctionSelectorItemPtr& Child : Item->Children)
			{
				if (Child->Object.IsValid() && Child->Object->GetName().Contains(SearchText.ToString()))
				{
					if (!FilteredItem)
					{
						FilteredItem = MakeShared<FBPE_FunctionSelectorItem>(*Item);
						FilteredItem->Children.Reset();
						FilteredItems.Add(FilteredItem);
						TreeView->SetItemExpansion(FilteredItem, true);
					}
					FilteredItem->Children.Add(Child);
				}
			}
		}
	}

	TreeView->RequestTreeRefresh();
}

void SBPE_FunctionSelector::OnSearchTextCommitted(const FText& InText, ETextCommit::Type InCommitType)
{
	if ((InCommitType == ETextCommit::Type::OnEnter) && FilteredItems.Num() > 0)
	{
		TreeView->SetSelection(FilteredItems[0], ESelectInfo::OnKeyPress);
		SetCurrentItem(FilteredItems[0]);
	}
}

void SBPE_FunctionSelector::SetCurrentItem(FBPE_FunctionSelectorItemPtr InItem) const
{
	if (InItem->ItemType == EBPE_FunctionSelectorItemType::AddNewFunction)
	{
		if (TOptional<FString> NewFunction = AddNewFunction.Execute())
		{
			PropertyHandle->SetValue(*NewFunction);
		}
		
		return;
	}
	
	check(InItem->ItemType == EBPE_FunctionSelectorItemType::MemberFunction || InItem->ItemType == EBPE_FunctionSelectorItemType::StaticFunction);
	PropertyHandle->SetValue(InItem->ItemType == EBPE_FunctionSelectorItemType::MemberFunction ? InItem->Object->GetName() : InItem->Object->GetPathName());
}

TSharedRef<ITableRow> SBPE_FunctionSelector::OnGenerateRow(FBPE_FunctionSelectorItemPtr InItem, const TSharedRef<STableViewBase>& InParent) const
{
	static const FSlateBrush* const IconBrush = FAppStyle::GetBrush(TEXT("Kismet.AllClasses.FunctionIcon"));
	static const FSlateBrush* const AddFunction = FAppStyle::Get().GetBrush("Icons.PlusCircle");
	return SNew(STableRow<TSharedPtr<FText>>, InParent)
	[
		SNew(SHorizontalBox)
		.ToolTipText(InItem->ToolTip)
		+SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SImage)
 			.Image(InItem->ItemType == EBPE_FunctionSelectorItemType::AddNewFunction ? AddFunction : IconBrush)
 			.Visibility(InItem->Children.IsEmpty() ? EVisibility::Visible : EVisibility::Collapsed)
		]
		+SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SSpacer).Size(4.0f).Visibility(EVisibility::Visible)
		]
		+SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(STextBlock)
			.Text(InItem->DisplayName.IsEmpty() && InItem->Object.IsValid() ? InItem->Object->GetDisplayNameText() : InItem->DisplayName)
			.HighlightText_Lambda([this]() { return SearchText; })
			.Font(InItem->Children.IsEmpty() ? FAppStyle::GetFontStyle(TEXT("Kismet.TypePicker.NormalFont")) : FAppStyle::GetFontStyle(TEXT("Kismet.TypePicker.CategoryFont")))
		]
 			
	];
}

void SBPE_FunctionSelector::RefreshFunctions()
{
	Items.Reset();

	const UClass* OwnerClass = MemberClass.Get(nullptr);
	if (OwnerClass)
	{
		const FBPE_FunctionSelectorItemPtr Owner = MakeShared<FBPE_FunctionSelectorItem>(OwnerClass);
		Owner->SetDisplayName(FText::Format(INVTEXT("{0} (Self)"), OwnerClass->GetDisplayNameText()));
		Items.Add(Owner);
	}

	auto IsMemberFunction = [OwnerClass](const UFunction* Function)
	{
		for (const UClass* Class = OwnerClass; Class; Class = Class->GetSuperClass())
		{
			if (Function->IsIn(Class))
			{
				return true;
			}
		}
		return false;
	};
		
	for (const UFunction* Function : TObjectRange<UFunction>())
	{
		const bool bIsMemberFunction = IsMemberFunction(Function);
		if (!Function->HasAnyFunctionFlags(FUNC_BlueprintCallable | FUNC_BlueprintPure) || !FunctionFilter.Execute(Function, bIsMemberFunction))
		{
			continue;
		}

		auto Item = MakeShared<FBPE_FunctionSelectorItem>(Function, bIsMemberFunction);
		if (bIsMemberFunction)
		{
			Items[0]->Children.Add(Item);
		}
		else
		{
			const FBPE_FunctionSelectorItemPtr* FoundOuter = Items.FindByPredicate([Function](const FBPE_FunctionSelectorItemPtr& Item) { return Item->Object == Function->GetOuterUClass(); });
			if (!FoundOuter)
			{
				FoundOuter = &Items.Add_GetRef(MakeShared<FBPE_FunctionSelectorItem>(Function->GetOuterUClass()));
			}

			FoundOuter->Get()->Children.Add(Item);
		}
	}

	static const FText AddFunctionName(INVTEXT("Add new function..."));
	static const FText AddFunctionTooltip(INVTEXT("Adds a new function with the correct signature to be used with this property."));
	Items[0]->Children.Add(MakeShared<FBPE_FunctionSelectorItem>(AddFunctionName, AddFunctionTooltip, EBPE_FunctionSelectorItemType::AddNewFunction));
}
