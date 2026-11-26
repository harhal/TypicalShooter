#include "Enemies/BuildEnemyStateTreeCommandlet.h"

#include "TypicalShooter.h"

#if WITH_EDITOR
#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/StateTreeAIComponentSchema.h"
#include "Enemies/EnemyStateTreeConditions.h"
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

namespace BuildEnemyStateTree
{
	struct FRoleState
	{
		EEnemyRole Role;
		const TCHAR* Name;
	};

	constexpr FRoleState RoleStates[] = {
		{EEnemyRole::Pressure, TEXT("Pressure")},
		{EEnemyRole::Flank, TEXT("Flank")},
		{EEnemyRole::Cover, TEXT("Cover")},
	};

	void AddGoToRole(UStateTreeState& From, const UStateTreeState& To, const EEnemyRole Role)
	{
		FStateTreeTransition& Transition = From.AddTransition(EStateTreeTransitionTrigger::OnTick, EStateTreeTransitionType::GotoState, &To);
		Transition.AddConditionWithOuter<FEnemyHasRoleCondition>(&From).GetInstanceData().Role = Role;
	}
}

void UBuildEnemyStateTreeCommandlet::AddRoleStates(UStateTreeState& Combat)
{
	using namespace BuildEnemyStateTree;
	TArray<UStateTreeState*> ByRole;
	for (const FRoleState& RoleState : RoleStates)
	{
		UStateTreeState& State = Combat.AddChildState(RoleState.Name);
		State.AddEnterCondition<FEnemyHasRoleCondition>().GetInstanceData().Role = RoleState.Role;
		State.AddTask<FEnemyKeepDistanceTask>(); // band and sector come from the role (director)
		ByRole.Add(&State);
	}
	// Not coordinated (no target): the task's own band, last so it is the fallback.
	UStateTreeState& Free = Combat.AddChildState(TEXT("Free"));
	Free.AddTask<FEnemyKeepDistanceTask>();

	for (int32 From = 0; From < ByRole.Num(); ++From)
	{
		for (int32 To = 0; To < ByRole.Num(); ++To)
		{
			if (To != From)
			{
				AddGoToRole(*ByRole[From], *ByRole[To], RoleStates[To].Role);
			}
		}
		FStateTreeTransition& ToFree = ByRole[From]->AddTransition(EStateTreeTransitionTrigger::OnTick, EStateTreeTransitionType::GotoState, &Free);
		ToFree.AddConditionWithOuter<FEnemyNotCoordinatedCondition>(ByRole[From]);
	}
	for (int32 To = 0; To < ByRole.Num(); ++To)
	{
		AddGoToRole(Free, *ByRole[To], RoleStates[To].Role);
	}
}

bool UBuildEnemyStateTreeCommandlet::BuildTree(UStateTree& Tree)
{
	const TSubclassOf<UStateTreeSchema> SchemaClass = UStateTreeAIComponentSchema::StaticClass();
	FStateTreeEditorModule& EditorModule = FStateTreeEditorModule::GetModule();

	UStateTreeEditorData* EditorData = NewObject<UStateTreeEditorData>(&Tree, EditorModule.GetEditorDataClass(SchemaClass.Get()), NAME_None, RF_Transactional);
	EditorData->Schema = NewObject<UStateTreeSchema>(EditorData, SchemaClass, NAME_None, RF_Transactional);
	EditorData->EditorSchema = NewObject<UStateTreeEditorSchema>(EditorData, EditorModule.GetEditorSchemaClass(SchemaClass.Get()), NAME_None, RF_Transactional);
	Tree.EditorData = EditorData;

	// Combat tracks and shoots; its children only position the enemy, one per role the director gives (plus Free when it
	// coordinates nothing). Transitions go between siblings, so Combat (and the shoot task's token) is not restarted.
	UStateTreeState& Combat = EditorData->AddRootState().AddChildState(TEXT("Combat"));
	Combat.AddTask<FEnemyTrackTargetTask>();
	Combat.AddTask<FEnemyShootTask>();
	AddRoleStates(Combat);

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
