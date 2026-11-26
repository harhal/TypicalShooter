#include "Enemies/BuildEnemyStateTreeCommandlet.h"

#include "TypicalShooter.h"

#if WITH_EDITOR
#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/StateTreeAIComponentSchema.h"
#include "Enemies/EnemyStateTreeTasks.h"
#include "Misc/PackageName.h"
#include "StateTree.h"
#include "StateTreeCompilerLog.h"
#include "StateTreeEditingSubsystem.h"
#include "StateTreeEditorData.h"
#include "StateTreeEditorModule.h"
#include "StateTreeEditorSchema.h"
#include "StateTreeState.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

bool UBuildEnemyStateTreeCommandlet::BuildTree(UStateTree& Tree)
{
	const TSubclassOf<UStateTreeSchema> SchemaClass = UStateTreeAIComponentSchema::StaticClass();
	FStateTreeEditorModule& EditorModule = FStateTreeEditorModule::GetModule();

	UStateTreeEditorData* EditorData = NewObject<UStateTreeEditorData>(&Tree, EditorModule.GetEditorDataClass(SchemaClass.Get()), NAME_None, RF_Transactional);
	EditorData->Schema = NewObject<UStateTreeSchema>(EditorData, SchemaClass, NAME_None, RF_Transactional);
	EditorData->EditorSchema = NewObject<UStateTreeEditorSchema>(EditorData, EditorModule.GetEditorSchemaClass(SchemaClass.Get()), NAME_None, RF_Transactional);
	Tree.EditorData = EditorData;

	UStateTreeState& Combat = EditorData->AddRootState().AddChildState(TEXT("Combat"));
	Combat.AddTask<FEnemyTrackTargetTask>();
	Combat.AddTask<FEnemyShootTask>();
	Combat.AddTask<FEnemyKeepDistanceTask>();

	FStateTreeCompilerLog Log;
	const bool bCompiled = UStateTreeEditingSubsystem::CompileStateTree(&Tree, Log);
	Log.DumpToLog(LogTypicalShooter);
	return bCompiled;
}
#endif

int32 UBuildEnemyStateTreeCommandlet::Main(const FString& Params)
{
#if WITH_EDITOR
	UPackage* Package = CreatePackage(AssetPath);
	Package->FullyLoad();
	const FName AssetName(FPackageName::GetLongPackageAssetName(AssetPath));
	UStateTree* Tree = FindObject<UStateTree>(Package, *AssetName.ToString());
	const bool bNew = Tree == nullptr;
	if (bNew)
	{
		Tree = NewObject<UStateTree>(Package, AssetName, RF_Public | RF_Standalone | RF_Transactional);
	}
	if (!BuildTree(*Tree))
	{
		UE_LOG(LogTypicalShooter, Error, TEXT("BuildEnemyStateTree: %s failed to compile"), AssetPath);
		return 1;
	}
	if (bNew)
	{
		FAssetRegistryModule::AssetCreated(Tree);
	}
	Package->MarkPackageDirty();

	const FString Filename = FPackageName::LongPackageNameToFilename(AssetPath, FPackageName::GetAssetPackageExtension());
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	if (!UPackage::SavePackage(Package, Tree, *Filename, SaveArgs))
	{
		UE_LOG(LogTypicalShooter, Error, TEXT("BuildEnemyStateTree: could not save %s"), *Filename);
		return 1;
	}
	UE_LOG(LogTypicalShooter, Display, TEXT("BuildEnemyStateTree: saved %s"), *Filename);
	return 0;
#else
	return 1;
#endif
}
