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
- Unreal MCP (Epic's experimental `ModelContextProtocol` plugin, editor-only) + toolsets
  (Editor, UMG, GameplayTags, StateTree, AutomationTest, LiveCoding, ConfigSettings). The editor
  auto-starts an HTTP MCP server at `http://127.0.0.1:8000/mcp` (set in
  `Config/DefaultEditorPerProjectUserSettings.ini`); Claude Code connects via `.mcp.json`
  (server `unreal-mcp`). Tools are only available while the editor is open.
- Claude Code plugin `unreal-engine-skills-for-claude-code@claude-plugins-official` (Epic, MIT) is
  enabled in `.claude/settings.json`: provides the `unreal-mcp` skill (how to drive the editor via
  `list_toolsets` / `describe_toolset` / `call_tool`) and a SessionStart hook that needs bash
  (Git Bash) on PATH.

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

- **Items**: `FInventoryItemManifest` = array of `TInstancedStruct<FItemFragment>`; look fragments up
  with `GetFragmentOfType<T>()` or by tag with `GetFragment<T>(Tag)`. Fragments: `FGrid_ItemFragment`
  (`Size`), `FInfo_ItemFragment` (`DisplayName`, `Description`, `Icon` — feeds the UI).
  `ManifestItem()` creates a `UInventoryItem` (UObject, replicated as a subobject).
- **Inventory**: `UInventoryComponent` owns `FInventoryArray` (FastArraySerializer of
  `{Item, FHhAxialCoord Location}`, linear lookups, one item per cell) and broadcasts
  added/removed/moved delegates on server and clients. Mutations (`AddItem/RemoveItem/MoveItem`) are
  **server-only**; there are no RPCs yet for client-initiated moves.
- **World**: `AItemsContainer` (implements `IPlayerInteractive` and `IItemsContainer_Model`, forwarding
  to its `InventoryComponent`) fills its inventory from `ItemsManifest` on the server, laying items out
  in a hex spiral. Two `BP_ItemsContainer` instances are placed in `Lvl_FirstPerson`.
- **Interaction**: `UInteractionComponent` on the character does an async sphere overlap on
  `ECC_GameTraceChannel2` ("PlayerInteraction") every tick; `PrimaryInteract` input on the controller.
- **UI (MVP)**: Model interfaces (`IInventoryItem_Model`, `IItemsContainer_Model`, implemented by
  `UInventoryItem` / `UInventoryComponent`) → UObject presenters → view interfaces implemented by
  Slate widgets (`SInventoryGrid` on top of `SHhGridPanel`, `SInventoryGridItem`). UMG entry point:
  `UItemsContainer_Presenter::CreatePresenter(Model)` → `UInventoryGridWidget::AssignPresenter`.
  HUD (`BP_HUD`) is spawned by `BP_FirstPersonPlayerController`.
- Custom content lives in `Content/Inventory` (note the folder typo `Bllueprints`).

## Current state (as of 2026-11-24)

C++ builds. Inventory core, replication and presenter/Slate wiring were fixed on 2026-11-24 but
**not yet verified in PIE**.

Open work:
- `BP_ContainerGrid` and `BP_InventoryGrid` still use the old BP-driven grid (deleted `GridPanel`
  `UHhGridPanelWidget` variable + `AddChildToGrid` nodes) and fail to compile. Needs editor work:
  replace with `UInventoryGridWidget` + `AssignPresenter(CreatePresenter(...))`.
- No item in the level has an `FInfo_ItemFragment` yet, so items show their object name only.
- Drag & drop between cells/containers: `SInventoryGridItem` has stub handlers; no server RPC for moves.

Verify blueprints headlessly with:
`UnrealEditor-Cmd.exe <uproject> -run=CompileAllBlueprints -unattended -nopause -NullRHI -stdout`

Game design (roguelike arena loop, weapons, enemies) has not been started; `Variant_Shooter` template
code is the current basis for combat/AI.
