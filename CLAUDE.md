# TypicalShooter

Roguelike arena shooter on Unreal Engine 5.8 (C++). The distinguishing feature is an
unusual inventory implementation: items are placed on a hexagonal grid (axial coordinates from the
`HarhalHexagonalGrid` plugin).
The game concept is still rough; details will be added as they settle.

Communicate with the user in Russian; code, identifiers and commit messages in English.

## Engine & build

- Engine: UE 5.8, installed at `C:\Program Files\Epic Games\UE_5.8`.
- Primary module: `TypicalShooter` (Runtime). Project plugin: `HarhalHexagonalGrid` (Runtime).
- Engine plugins in use: GameplayAbilities (GAS), GASToolsets, StateTree, GameplayStateTree,
  TargetingSystem, AbilitySystemGameFeatureActions, ModelingToolsEditorMode.

Build editor target from the command line (close the editor first or use Live Coding instead):

```powershell
& "C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" TypicalShooterEditor Win64 Development -Project="$PWD\TypicalShooter.uproject" -WaitMutex
```

Regenerate IDE project files (`.sln` / `.slnx` are gitignored):

```powershell
& "C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" -projectfiles -project="$PWD\TypicalShooter.uproject" -game -engine
```

## Source layout

- `Source/TypicalShooter/` — template base classes (`TypicalShooterGameMode`, `PlayerController`,
  `CameraManager`) from the UE First Person template.
- `Source/TypicalShooter/Variant_Shooter/`, `Variant_Horror/` — template variants (weapons, AI via
  StateTree/EQS, pickups, UI). Template code; modify freely but treat as a starting point.
- `Source/TypicalShooter/Public|Private/` — custom gameplay code:
  - Inventory core: `InventoryComponent`, `InventoryArray` (likely FastArray replication),
    `InventoryItem`, `InventoryItemManifest`, `ItemFragment`, `ItemsContainer`.
  - `InteractionComponent`, `TS_GAS`, `TypicalShooterCharacter`.
  - `UI/` — inventory UI in MVP style (`*_Model`, `*_Presenter`, `*_View`), plus Slate widgets
    (`UI/Slate/InventoryGrid*`) wrapped by UMG (`UI/Widget/InventoryGridWidget`).
- `Plugins/HarhalHexagonalGrid/` — user's own hex-grid library: axial coords (`HhAxialCoord`),
  grid math (`HhGridFunctions`), Slate panel (`HhGridPanel`) + UMG wrapper (`HhGridPanelWidget`),
  `HhGridPlane`.
- `Content/` — mostly template content (`FirstPerson`, `Variant_*`, `Weapons`, `Characters`,
  `LevelPrototyping`, `Input`); custom content lives in `Content/Inventory`.
  Uses One File Per Actor (`__ExternalActors__`, `__ExternalObjects__`).

Note: `Build.cs` adds `PublicIncludePaths` for the template folders, so headers there are
included by bare filename.

## Conventions

- Follow Unreal coding standard (prefixes `U/A/F/S/E/I`, `UPROPERTY`/`UFUNCTION` reflection).
- Prefer C++ for systems; Blueprints for content/tuning.
- Keep the hex-grid plugin independent of game code (no dependency on `TypicalShooter`).

## Git

- `.uasset` / `.umap` and binary media go through Git LFS (see `.gitattributes`); assets are
  `lockable`.
- Never commit `Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/`, generated `.sln`,
  IDE folders or `Content/Developers/`.

## Architecture (custom code)

- **Items**: `FInventoryItemManifest` = array of `TInstancedStruct<FItemFragment>` (fragments are
  identified by `FGameplayTag`; `FGrid_ItemFragment` has `Size`). `ManifestItem()` creates a
  `UInventoryItem` (UObject, replicated as a subobject).
- **Inventory**: `UInventoryComponent` owns `FInventoryArray` (FastArraySerializer of
  `{Item, FHhAxialCoord Location}` + `CoordsMap` lookup) and broadcasts added/removed/moved delegates.
- **World**: `AItemsContainer` (implements `IPlayerInteractive`) fills its inventory from
  `ItemsManifest` on the server, laying items out in a hex spiral. Two `BP_ItemsContainer` instances
  are placed in `Lvl_FirstPerson`.
- **Interaction**: `UInteractionComponent` on the character does an async sphere overlap on
  `ECC_GameTraceChannel2` ("PlayerInteraction") every tick; `PrimaryInteract` input on the controller.
- **UI (MVP)**: Model interfaces (`IInventoryItem_Model`, `IItemsContainer_Model`, implemented by
  `UInventoryItem` / `UInventoryComponent`) → UObject presenters → view interfaces implemented by
  Slate widgets (`SInventoryGrid` on top of `SHhGridPanel`, `SInventoryGridItem`). UMG entry point is
  `UInventoryGridWidget::AssignPresenter`. HUD (`BP_HUD`) is spawned by `BP_FirstPersonPlayerController`.
- Custom content lives in `Content/Inventory` (note the folder typo `Bllueprints`).

## Current state (as of 2026-11-24)

Work stopped (2026-09-30) in the middle of wiring the inventory UI. Last successful build was
2026-09-11; the current tree **does not compile**.

Known problems, roughly in fixing order:
1. Build error: `SInventoryGridItem::SetName` passes `*Name` to `STextBlock::SetText` (needs `FText`).
   `SetIcon` stores a pointer to a by-value local brush (dangling).
2. `FInventoryArray::Owner` is never assigned → `AddItem` returns early; containers stay empty.
3. `FInventoryArray_Entry::Item` / `Location` are not `UPROPERTY` → nothing replicates.
   (`TStrongObjectPtr` should be a `UPROPERTY() TObjectPtr`.)
4. `FInventoryArray::RemoveItem(idx)` indexes `Entries[Idx]` after `RemoveAtSwap` (breaks when removing
   the last element); `MoveItem` uses `CoordsMap[Location]` on a missing key (assert).
5. `UInventoryComponent::Get*Delegate()` and `IItemsContainer_Model` return delegates **by value** →
   presenter subscriptions bind to copies and never fire. Presenter `OnItemAdded/Removed/Moved` are stubs.
6. `SInventoryGrid::AddInventoryItem` wraps a panel-owned `FSlot*` in `MakeShareable` (double free);
   `RemoveInventoryItem` doesn't remove the slot from the panel.
7. `UInventoryItem` model getters return interface defaults (no data from fragments yet).
   `FInventoryItemManifest::GetFragment` iterates by value and returns a pointer into a copy.
8. `BP_ContainerGrid` references a removed `CreatePresenter` function and a deleted `GridPanel`
   variable (editor warnings on load).
9. Plugin: `UHhGridPanelWidget::ReleaseSlateResources` dereferences `SlateGridPanel` without a check;
   UMG classes lack the `HARHALHEXAGONALGRID_API` export.

Game design (roguelike arena loop, weapons, enemies) has not been started; `Variant_Shooter` template
code is the current basis for combat/AI.
