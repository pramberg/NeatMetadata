// Copyright Viktor Pramberg. All Rights Reserved.
#include "BPE_Module.h"

#include "BlueprintEditorModule.h"
#include "BPE_VariableDetailCustomization.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FBlueprintPropertyExtensionsModule"

DEFINE_LOG_CATEGORY(LogBlueprintPropertyExtensions);

class FBPE_Module : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		FBlueprintEditorModule& BlueprintEditorModule = FModuleManager::GetModuleChecked<FBlueprintEditorModule>("Kismet");
		BlueprintVariableCustomizationHandle = BlueprintEditorModule.RegisterVariableCustomization(FProperty::StaticClass(), FOnGetVariableCustomizationInstance::CreateStatic(&BPE_VariableDetailCustomization::MakeInstance));
	}
	
	virtual void ShutdownModule() override
	{
		if (FBlueprintEditorModule* BlueprintEditorModule = FModuleManager::GetModulePtr<FBlueprintEditorModule>("Kismet"))
		{
			BlueprintEditorModule->UnregisterVariableCustomization(FProperty::StaticClass(), BlueprintVariableCustomizationHandle);
		}
	}

private:
	FDelegateHandle BlueprintVariableCustomizationHandle;
};

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBPE_Module, BlueprintPropertyExtensions)