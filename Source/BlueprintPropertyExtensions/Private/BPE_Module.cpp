// Copyright Epic Games, Inc. All Rights Reserved.

#include "BPE_Module.h"

#include "BlueprintEditorModule.h"
#include "BPE_VariableDetailCustomization.h"

#define LOCTEXT_NAMESPACE "FBlueprintPropertyExtensionsModule"

void FBPE_Module::StartupModule()
{
	FBlueprintEditorModule& BlueprintEditorModule = FModuleManager::GetModuleChecked<FBlueprintEditorModule>("Kismet");
	BlueprintVariableCustomizationHandle = BlueprintEditorModule.RegisterVariableCustomization(FProperty::StaticClass(), FOnGetVariableCustomizationInstance::CreateStatic(&BPE_VariableDetailCustomization::MakeInstance));
}

void FBPE_Module::ShutdownModule()
{
	if (FBlueprintEditorModule* BlueprintEditorModule = FModuleManager::GetModulePtr<FBlueprintEditorModule>("Kismet"))
	{
		BlueprintEditorModule->UnregisterVariableCustomization(FProperty::StaticClass(), BlueprintVariableCustomizationHandle);
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBPE_Module, BlueprintPropertyExtensions)