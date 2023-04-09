// Copyright Viktor Pramberg. All Rights Reserved.
#include "NeatMetadataModule.h"
#include "NeatMetadataDetailCustomization.h"

#include "BlueprintEditorModule.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogNeatMetadata);

class FNeatMetadataModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		FBlueprintEditorModule& BlueprintEditorModule = FModuleManager::GetModuleChecked<FBlueprintEditorModule>("Kismet");
		BlueprintVariableCustomizationHandle = BlueprintEditorModule.RegisterVariableCustomization(FProperty::StaticClass(), FOnGetVariableCustomizationInstance::CreateStatic(&FNeatMetadataDetailCustomization::MakeInstance));
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
	
IMPLEMENT_MODULE(FNeatMetadataModule, NeatMetadata)