// Copyright Viktor Pramberg. All Rights Reserved.


#include "SNeatInterfaceSelector.h"

#include "ClassViewerFilter.h"
#include "ClassViewerModule.h"
#include "DetailLayoutBuilder.h"

namespace
{
	// Class filter that only shows interfaces.
	class FNeatInterfaceClassFilter : public IClassViewerFilter
	{
	public:
		virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
		{
			return InClass->HasAnyClassFlags(EClassFlags::CLASS_Interface);
		}

		virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef<const IUnloadedBlueprintData> InClass, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
		{
			return InClass->HasAnyClassFlags(EClassFlags::CLASS_Interface);
		}
	};
}

void SNeatInterfaceSelector::Construct(const FArguments&, TSharedRef<IPropertyHandle> InPropertyHandle)
{
	PropertyHandle = InPropertyHandle;

	ChildSlot
	[
		SAssignNew(ComboButton, SComboButton)
		.OnGetMenuContent(this, &SNeatInterfaceSelector::GetMenuContent)
		.ButtonContent()
		[
			SNew(STextBlock)
			.Text(this, &SNeatInterfaceSelector::GetButtonText)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
	];
}

TSharedRef<SWidget> SNeatInterfaceSelector::GetMenuContent()
{
	FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");
	FClassViewerInitializationOptions Options;
	Options.bShowUnloadedBlueprints = true;
	Options.bShowNoneOption = true;
	Options.bAllowViewOptions = false;
	Options.NameTypeToDisplay = EClassViewerNameTypeToDisplay::DisplayName;
	Options.PropertyHandle = PropertyHandle;
	Options.ClassFilters.Add(MakeShared<FNeatInterfaceClassFilter>());

	return SNew(SBox)
		.WidthOverride(280)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			.MaxHeight(500)
			[
				ClassViewerModule.CreateClassViewer(Options, FOnClassPicked::CreateSP(this, &SNeatInterfaceSelector::OnClassPicked))
			]
		];
}

void SNeatInterfaceSelector::OnClassPicked(UClass* InClass) const
{
	if (GetClass() != InClass)
	{
		PropertyHandle->SetValue(InClass);
	}
	
	ComboButton->SetIsOpen(false);
}

UClass* SNeatInterfaceSelector::GetClass() const
{
	UObject* Obj;
	PropertyHandle->GetValue(Obj);
	return Cast<UClass>(Obj);
}

FText SNeatInterfaceSelector::GetButtonText() const
{
	return GetClass() ? GetClass()->GetDisplayNameText() : INVTEXT("None");
}
