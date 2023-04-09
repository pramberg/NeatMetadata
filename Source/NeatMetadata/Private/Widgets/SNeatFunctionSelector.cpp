// Copyright Viktor Pramberg. All Rights Reserved.
#include "SNeatFunctionSelector.h"
#include "Widgets/Input/SSearchBox.h"
#include "SListViewSelectorDropdownMenu.h"
#include "Styling/SlateIconFinder.h"

enum class ENeatFunctionSelectorItemType : uint8
{
	None,
	Class,
	MemberFunction,
	StaticFunction,
	AddNewFunction,
};

struct FNeatFunctionSelectorItem
{
	explicit FNeatFunctionSelectorItem(const UClass* InClass) :
		Object(InClass),
		ItemType(ENeatFunctionSelectorItemType::Class),
		ToolTip(InClass ? InClass->GetToolTipText() : FText())
	{}
	
	explicit FNeatFunctionSelectorItem(const UFunction* InFunction, const bool bIsMemberFunction) :
		Object(InFunction),
		ItemType(bIsMemberFunction ? ENeatFunctionSelectorItemType::MemberFunction : ENeatFunctionSelectorItemType::StaticFunction),
		ToolTip(InFunction ? InFunction->GetToolTipText() : FText()),
		Value(bIsMemberFunction ? InFunction->GetName() : InFunction->GetPathName()),
		Icon(FAppStyle::GetBrush(TEXT("Kismet.AllClasses.FunctionIcon")))
	{}

	explicit FNeatFunctionSelectorItem(const FText InText, const FText InToolTip = FText(), ENeatFunctionSelectorItemType InItemType = ENeatFunctionSelectorItemType::None) :
		ItemType(InItemType),
		DisplayName(InText),
		ToolTip(InToolTip),
		Icon(FSlateIconFinder::FindIconBrushForClass(nullptr))
	{}

	void SetDisplayName(const FText& InText)
	{
		DisplayName = InText;
	}

	void SetIcon(const FSlateBrush* InIcon)
	{
		Icon = InIcon;
	}

protected:
	TWeakObjectPtr<const UStruct> Object = nullptr;
	ENeatFunctionSelectorItemType ItemType = ENeatFunctionSelectorItemType::None;
	FText DisplayName;
	FText ToolTip;
	FString Value;
	const FSlateBrush* Icon = nullptr;
	TArray<FNeatFunctionSelectorItemPtr> Children;

	friend class SNeatFunctionSelector;
};

void SNeatFunctionSelector::Construct(const FArguments& InArgs, TSharedRef<IPropertyHandle> InPropertyHandle)
{
	PropertyHandle = InPropertyHandle;

	FunctionFilter = InArgs._FunctionFilter;
	AddNewFunction = InArgs._AddNewFunction;
	MemberClass = InArgs._MemberClass;
	
	ChildSlot
	[
		SAssignNew(ComboButton, SComboButton)
		.OnGetMenuContent(this, &SNeatFunctionSelector::OnGetMenuContent)
		.ButtonContent()
		[
			SNew(STextBlock).Text(this, &SNeatFunctionSelector::GetText)
		]
	];
}

FText SNeatFunctionSelector::GetText() const
{
	FString Value;
	PropertyHandle->GetValue(Value);
	if (const int32 Index = Value.Find(TEXT(".")))
	{
		Value = Value.RightChop(Index + 1);
	}
	return FText::FromString(Value);
}

TSharedRef<SWidget> SNeatFunctionSelector::OnGetMenuContent()
{
	RefreshFunctions();

	SearchText = FText();
	FilteredItems.Reset();
	FilteredItems.Append(Items);

	TSharedPtr<SSearchBox> SearchBox;
	SAssignNew(SearchBox, SSearchBox)
	.HintText(INVTEXT("Search"))
	.OnTextChanged(this, &SNeatFunctionSelector::OnSearchTextChanged)
	.OnTextCommitted(this, &SNeatFunctionSelector::OnSearchTextCommitted);
	
	SAssignNew(TreeView, SNeatFunctionSelectorTreeView)
	.TreeItemsSource(&FilteredItems)
	.SelectionMode(ESelectionMode::Single)
	.ItemHeight(32)
	.OnSelectionChanged(this, &SNeatFunctionSelector::OnSelectionChanged)
	.OnGetChildren_Lambda([this](FNeatFunctionSelectorItemPtr InParent, TArray<FNeatFunctionSelectorItemPtr>& OutChildren) { OutChildren = InParent->Children; })
	.OnGenerateRow(this, &SNeatFunctionSelector::OnGenerateRow);

	ComboButton->SetMenuContentWidgetToFocus(SearchBox);
	if (!FilteredItems.IsEmpty())
	{
		TreeView->SetItemExpansion(FilteredItems[1], true);
	}
	
	return SNew(SListViewSelectorDropdownMenu<FNeatFunctionSelectorItemPtr>, SearchBox, TreeView)
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

void SNeatFunctionSelector::OnSelectionChanged(FNeatFunctionSelectorItemPtr InSelection, ESelectInfo::Type InSelectInfo) const
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

void SNeatFunctionSelector::OnSearchTextChanged(const FText& ChangedText)
{
	SearchText = ChangedText;

	FilteredItems.Reset();

	if (SearchText.IsEmpty())
	{
		FilteredItems.Append(Items);
		
		if (!FilteredItems.IsEmpty())
		{
			TreeView->SetItemExpansion(FilteredItems[1], true);
		}
	}
	else
	{
		for (const FNeatFunctionSelectorItemPtr& Item : Items)
		{
			FNeatFunctionSelectorItemPtr FilteredItem = nullptr;
			for (const FNeatFunctionSelectorItemPtr& Child : Item->Children)
			{
				if (Child->Object.IsValid() && Child->Object->GetName().Contains(SearchText.ToString()))
				{
					if (!FilteredItem)
					{
						FilteredItem = MakeShared<FNeatFunctionSelectorItem>(*Item);
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

void SNeatFunctionSelector::OnSearchTextCommitted(const FText& InText, ETextCommit::Type InCommitType)
{
	if ((InCommitType == ETextCommit::Type::OnEnter) && FilteredItems.Num() > 0)
	{
		TreeView->SetSelection(FilteredItems[0], ESelectInfo::OnKeyPress);
		SetCurrentItem(FilteredItems[0]);
	}
}

void SNeatFunctionSelector::SetCurrentItem(FNeatFunctionSelectorItemPtr InItem) const
{
	if (InItem->ItemType == ENeatFunctionSelectorItemType::AddNewFunction)
	{
		if (TOptional<FString> NewFunction = AddNewFunction.Execute())
		{
			PropertyHandle->SetValue(*NewFunction);
		}
		
		return;
	}
	
	PropertyHandle->SetValue(InItem->Value);

	ComboButton->SetIsOpen(false);
}

TSharedRef<ITableRow> SNeatFunctionSelector::OnGenerateRow(FNeatFunctionSelectorItemPtr InItem, const TSharedRef<STableViewBase>& InParent) const
{
	return SNew(STableRow<TSharedPtr<FText>>, InParent)
	[
		SNew(SHorizontalBox)
		.ToolTipText(InItem->ToolTip)
		+SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0.0f, 3.0f, 6.0f, 3.0f)
		[
			SNew(SImage)
			.Image(InItem->Icon)
			.Visibility(InItem->Children.IsEmpty() ? EVisibility::Visible : EVisibility::Collapsed)
		]
		+SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.VAlign(VAlign_Center)
		.Padding(0.0f, 3.0f, 6.0f, 3.0f)
		[
			SNew(STextBlock)
			.Text(InItem->DisplayName.IsEmpty() && InItem->Object.IsValid() ? InItem->Object->GetDisplayNameText() : InItem->DisplayName)
			.HighlightText_Lambda([this]() { return SearchText; })
			.Font(InItem->Children.IsEmpty() ? FAppStyle::GetFontStyle(TEXT("NormalFont")) : FAppStyle::GetFontStyle(TEXT("NormalFontItalic")))
		]
 			
	];
}

void SNeatFunctionSelector::RefreshFunctions()
{
	Items.Reset();

	{
		Items.Add(MakeShared<FNeatFunctionSelectorItem>(INVTEXT("None")));
	}

	const UClass* OwnerClass = MemberClass.Get(nullptr);
	FNeatFunctionSelectorItemPtr OwnerItem;
	if (OwnerClass)
	{
		OwnerItem = MakeShared<FNeatFunctionSelectorItem>(OwnerClass);
		OwnerItem->SetDisplayName(FText::Format(INVTEXT("{0} (Self)"), OwnerClass->GetDisplayNameText()));
		Items.Add(OwnerItem);
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

		auto Item = MakeShared<FNeatFunctionSelectorItem>(Function, bIsMemberFunction);
		if (bIsMemberFunction && OwnerItem)
		{
			OwnerItem->Children.Add(Item);
		}
		else
		{
			const FNeatFunctionSelectorItemPtr* FoundOuter = Items.FindByPredicate([Function](const FNeatFunctionSelectorItemPtr& Item) { return Item->Object == Function->GetOuterUClass(); });
			if (!FoundOuter)
			{
				FoundOuter = &Items.Add_GetRef(MakeShared<FNeatFunctionSelectorItem>(Function->GetOuterUClass()));
			}

			FoundOuter->Get()->Children.Add(Item);
		}
	}

	if (OwnerItem)
	{
		static const FText AddFunctionName(INVTEXT("Add new function..."));
		static const FText AddFunctionTooltip(INVTEXT("Adds a new function with the correct signature to be used with this property."));
		const auto Item = MakeShared<FNeatFunctionSelectorItem>(AddFunctionName, AddFunctionTooltip, ENeatFunctionSelectorItemType::AddNewFunction);
		Item->SetIcon(FAppStyle::GetBrush(TEXT("Icons.PlusCircle")));
		OwnerItem->Children.Add(Item);
	}
}
