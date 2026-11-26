// Editor commandlet that (re)builds the enemy behaviour StateTree asset from code, so the tree stays reproducible:
//   UnrealEditor-Cmd.exe <uproject> -run=BuildEnemyStateTree -unattended -nopause
// Tree: Root -> Combat {Enemy: Track Target, Enemy: Shoot When Visible, Enemy: Keep Distance} (AI component schema).

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "BuildEnemyStateTreeCommandlet.generated.h"

class UStateTree;

UCLASS()
class TYPICALSHOOTER_API UBuildEnemyStateTreeCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	/** Package of the generated asset. */
	static constexpr const TCHAR* AssetPath = TEXT("/Game/Enemies/AI/ST_Enemy");

	virtual int32 Main(const FString& Params) override;

#if WITH_EDITOR
	/** Replaces the tree's editor data with the enemy behaviour and compiles it. Returns false on compile errors. */
	static bool BuildTree(UStateTree& Tree);

private:
	/** Pressure / Flank / Cover (enter condition: the director's role) and Free under Combat, with sibling transitions. */
	static void AddRoleStates(class UStateTreeState& Combat);
#endif
};
