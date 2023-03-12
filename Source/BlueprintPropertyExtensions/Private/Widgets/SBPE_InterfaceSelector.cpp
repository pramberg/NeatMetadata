// Copyright Viktor Pramberg. All Rights Reserved.


#include "SBPE_InterfaceSelector.h"

#include "ClassViewerFilter.h"
#include "ClassViewerModule.h"
#include "DetailLayoutBuilder.h"

namespace
{
	// Class filter that only shows interfaces.
	class FBPE_InterfaceClassFilter : public IClassViewerFilter
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

void SBPE_InterfaceSelector::Construct(const FArguments&, TSharedRef<IPropertyHandle> InPropertyHandle)
{
	PropertyHandle = InPropertyHandle;

	ChildSlot
	[
		SNew(SComboButton)
		.OnGetMenuContent(this, &SBPE_InterfaceSelector::GetMenuContent)
		.ButtonContent()
		[
			SNew(STextBlock)
			.Text(this, &SBPE_InterfaceSelector::GetButtonText)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
	];
}

TSharedRef<SWidget> SBPE_InterfaceSelector::GetMenuContent()
{
	FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");
	FClassViewerInitializationOptions Options;
	Options.bShowUnloadedBlueprints = true;
	Options.bShowNoneOption = true;
	Options.bAllowViewOptions = false;
	Options.NameTypeToDisplay = EClassViewerNameTypeToDisplay::DisplayName;
	Options.InitiallySelectedClass = GetClass();
	Options.ClassFilters.Add(MakeShared<FBPE_InterfaceClassFilter>());

	return SNew(SBox)
		.WidthOverride(280)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			.MaxHeight(500)
			[
				ClassViewerModule.CreateClassViewer(Options, FOnClassPicked::CreateSP(this, &SBPE_InterfaceSelector::OnClassPicked))
			]
		];
}

void SBPE_InterfaceSelector::OnClassPicked(UClass* InClass) const
{
	if (GetClass() != InClass)
	{
		PropertyHandle->SetValue(InClass);
	}
}

UClass* SBPE_InterfaceSelector::GetClass() const
{
	UObject* Obj;
	PropertyHandle->GetValue(Obj);
	return Cast<UClass>(Obj);
}

FText SBPE_InterfaceSelector::GetButtonText() const
{
	return GetClass() ? GetClass()->GetDisplayNameText() : INVTEXT("None");
}
