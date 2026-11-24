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

### Editing assets through Unreal MCP (learned in practice)
- Widget trees: `UMGToolSet` (`GetWidgets`, `AddWidget`, `WrapWidgets`, `ToggleWidgetAsVariable`,
  `CompileWidgetBlueprint`); widget/slot properties via `ObjectTools.get/set_properties`.
- Graphs: `BlueprintTools.read_graph_dsl` to read; edit with `find_nodes` / `get_node_infos` /
  `delete_node` / `create_node` / `connect_pins`. Nodes with stale pins can't be refreshed — delete,
  compile (so new variables enter the class), then recreate. A new widget variable's getter may be
  missing from `find_node_types`, but `create_node "Variables|<BP>|Get<Name>"` still works.
- `FInstancedStruct` JSON: `{"_structType": "/Script/Module.Struct", "field": ...}` (camelCase).
  Arrays are applied as diffs: `reset_properties` first, then set the full array.
- Saving: `AssetTools.save_assets` with package paths. Level-placed actors (One File Per Actor) only
  save via `save_assets` with an empty list (= all dirty packages) — check `git status` afterwards.
- Runtime check: `EditorAppToolset.StartPIE/StopPIE` + `LogsToolset.GetLogEntries`;
  C++ changes while the editor is open: `LiveCodingToolset.CompileLiveCoding`.

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
- `Plugins/HarhalHexagonalGrid/` — **git submodule** (separate repository): user's own hex-grid
  library: axial coords (`HhAxialCoord`), grid math (`HhGridFunctions`), Slate panel (`HhGridPanel`)
  + UMG wrapper (`HhGridPanelWidget`), `HhGridPlane`. See "Git" for the submodule workflow.
- `Content/` — mostly template content (`FirstPerson`, `Variant_*`, `Weapons`, `Characters`,
  `LevelPrototyping`, `Input`); custom content lives in `Content/Inventory`.
  Uses One File Per Actor (`__ExternalActors__`, `__ExternalObjects__`).

Note: `Build.cs` adds `PublicIncludePaths` for the template folders, so headers there are
included by bare filename.

## Conventions

- Follow Unreal coding standard (prefixes `U/A/F/S/E/I`, `UPROPERTY`/`UFUNCTION` reflection).
- Prefer C++ for systems; Blueprints for content/tuning.
- Keep the hex-grid plugin independent of game code (no dependency on `TypicalShooter`).

## Feature documentation

- `Docs/Features/` holds living per-feature docs (in Russian): `README.md` is the index with a status
  table, `_Template.md` the template for new features.
- Keep them in sync: when a change alters a feature's behaviour, controls, content setup, tests or open
  tasks, update that feature's doc (status, sections, "Сверено с кодом" date) in the same commit.
  A new feature gets a new doc from the template plus a row in the index.

## Testing (TDD)

- Work test-first: for any new behaviour or bug fix, write a failing automation test first, run it
  and see it fail for the expected reason, then implement until it passes.
- Tests live in `Source/TypicalShooter/Private/Tests/` (plugin: `Plugins/<Plugin>/Source/<Module>/Private/Tests/`),
  named `TypicalShooter.<Area>.<Case>`, guarded by `WITH_DEV_AUTOMATION_TESTS`.
- Pure logic (hex math, inventory array, fragments, presenters): fast `IMPLEMENT_SIMPLE_AUTOMATION_TEST`
  without a world. Gameplay / UI flow: PIE tests like `TypicalShooter.Inventory.UIFlow` (start PIE,
  feed keys via `UGameViewportClient::InputKey`); keep these for end-to-end scenarios.
- Run via Unreal MCP `AutomationTestToolset` (`DiscoverTests` → `RunTests` → `GetTestResults`);
  the whole `TypicalShooter.` + `HarhalHexagonalGrid.` suites must pass before a commit.
  Existing suites: `TypicalShooter.Inventory.Core.*` (bare game world, no PIE; shared fixture in
  `Private/Tests/InventoryTestFixture.h`), `TypicalShooter.Inventory.UI.*` (presenter + Slate grid /
  info view without a window), `TypicalShooter.Inventory.UIFlow` (PIE), `HarhalHexagonalGrid.*` (pure math).
- Headless alternative (editor closed, also runs the PIE test):
  `UnrealEditor-Cmd.exe <uproject> "-ExecCmds=Automation RunTests TypicalShooter.+HarhalHexagonalGrid.;Quit" -unattended -nopause -NullRHI -abslog=<file>`
  and grep the log for `Test Completed. Result=`.
- Building while *any* editor with Live Coding runs fails ("Unable to build while Live Coding is
  active"), even for another checkout/worktree. Copying a file back with `Copy-Item` keeps its old
  timestamp, so UBT may skip it — touch it first.
- New test files / UFUNCTIONs need a full build with the editor closed; edits to existing test
  bodies can go through `LiveCodingToolset.CompileLiveCoding`.
- Unity builds merge .cpp files: no file-scope `using namespace` or short global names in tests.
- Gotcha: `TInstancedStruct<FItemFragment>::Make(DerivedLvalue)` picks the variadic overload and
  slices to the base type — always write `Make<FDerived>(Value)`.
- Never weaken or delete a failing test to make it pass; if behaviour intentionally changes, update
  the test first.

## Git

- `.uasset` / `.umap` and binary media go through Git LFS (see `.gitattributes`). Not `lockable`:
  there is no LFS lock server, and lockable files are checked out read-only (the editor can't save).
- Never commit `Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/`, generated `.sln`,
  IDE folders or `Content/Developers/`.

### HarhalHexagonalGrid submodule
- `Plugins/HarhalHexagonalGrid` is a submodule with its own history (no Git LFS there; the icon is a
  plain binary). Clone this project with `git clone --recursive`, or run
  `git submodule update --init` after cloning.
- Changing the plugin is two commits: commit (and push) inside `Plugins/HarhalHexagonalGrid` first,
  then commit the updated submodule pointer in this repository. Never leave the main repo pointing at
  a plugin commit that only exists locally once a real remote is used.
- Plugin commit messages describe plugin changes only; game changes go to this repository.
- Remotes (public, GitHub account `harhal`; `gh` CLI at `C:\Program Files\GitHub CLI\gh.exe`):
  project https://github.com/harhal/TypicalShooter, plugin https://github.com/harhal/HarhalHexagonalGrid.
  Push the plugin before pushing a main-repo commit that points at a new plugin commit.

## Architecture (custom code)

- **Items**: `FInventoryItemManifest` = array of `TInstancedStruct<FItemFragment>`; look fragments up
  with `GetFragmentOfType<T>()` or by tag with `GetFragment<T>(Tag)`. Fragments: `FGrid_ItemFragment`
  (`Size`), `FInfo_ItemFragment` (`DisplayName`, `Description`, `Icon` — feeds the UI).
  `ManifestItem()` creates a `UInventoryItem` (UObject, replicated as a subobject).
- **Inventory**: `UInventoryComponent` owns `FInventoryArray` (FastArraySerializer of
  `{Item, FHhAxialCoord Location}`, linear lookups, one item per cell) and broadcasts
  added/removed/moved delegates on server and clients. The grid is a hex of `GridRadius` (default 2 =
  19 cells; `IsLocationInBounds`, `FindFreeLocation` in spiral order). Mutations
  (`AddItem/RemoveItem/MoveItem/TransferItem/TransferItemToFreeLocation`) are **server-only**.
  `TransferItem` moves within or between inventories; an occupied target cell swaps the two items;
  between inventories the item is re-created from its manifest (items have no other state yet).
  `FInventoryArray`'s copy operations deliberately skip its `Owner` (BP actors copy component
  properties from the CDO; regression test `Core.OwnerSurvivesArchetypeCopy`).
- **Layout events** (`InventoryLayout.h`): besides the per-item delegates (used by the UI presenter, may see
  intermediate states), `UInventoryComponent::GetLayoutChangedDelegate()` / BP `OnLayoutChanged` fire **once
  per atomic operation**, when the inventory is consistent, with an `FInventoryLayoutChange` = placement
  changes + per-item neighbour changes (hex distance 1, `GainedNeighbors` / `LostNeighbors`, reported from both
  sides). Computed by diffing against the last reported layout (`InventoryLayout::Diff`). Server: every
  mutation opens a `FScopedLayoutBatch` (transfer/swap = one event per inventory); wrap multi-step operations
  explicitly (`ServerTakeItems` does). Clients: flushed in `FInventoryArray::PostReplicatedReceive`.
  Queries: `GetNeighborItems(Location)`, `GetItemNeighbors(Item)`. Item adjacency will drive gameplay later.
- **Client requests**: UI calls `IItemsContainer_Model::RequestTransferItem` → `UInventoryComponent`
  routes it to the first local `ATypicalShooterPlayerController::RequestTransferItem` →
  `ServerTransferItem` RPC (validates `CanAccessInventory`: own pawn's inventory, or an
  `AItemsContainer` within `SphereCollision` radius + `AccessRangeTolerance`). `TakeSelectedItem` /
  `TakeAllFromOpenedContainer` → `ServerTakeItems` (to the pawn's first free cells).
  `ATypicalShooterCharacter` has its own `InventoryComponent`.
- **World**: `AItemsContainer` (implements `IPlayerInteractive` and `IItemsContainer_Model`, forwarding
  to its `InventoryComponent`) fills its inventory from `ItemsManifest` on the server, laying items out
  in a hex spiral. Two `BP_ItemsContainer` instances are placed in `Lvl_FirstPerson`.
- **Interaction**: `UInteractionComponent` on the character does an async sphere overlap on
  `ECC_GameTraceChannel2` ("PlayerInteraction") every tick.
- **Inventory screen flow** (all in `ATypicalShooterPlayerController`, client-local UI):
  - `IA_ToggleInventory` (**Tab**) → `OpenInventory` / `CloseInventory`. Open = game+UI input mode
    with cursor; closed = game-only input (also applied in `BeginPlay`, which gives the viewport focus).
  - `IA_Interact` (**E**) → `PrimaryInteract`: nearest `IPlayerInteractive` overlap → `Interact`; for an
    `AItemsContainer` it opens the inventory *and* the container. E again closes both; leaving
    interaction range closes only the container; closing the inventory closes the container too.
  - BP events implemented in `BP_FirstPersonPlayerController`: `OnInventoryOpened/Closed` show/hide
    `HUD.InventoryScreen` (grid panel with `BP_InventoryGrid`, `BP_ContainerGrid`, `BP_ItemInfo`;
    collapsed by default); `OnItemsContainerOpened/Closed` → `HUD.SetContainer` + show/hide
    `HUD.BP_ContainerGrid`. `BP_InventoryGrid`'s Close button calls `CloseInventory`.
  - Esc can't be used for UI in PIE (it ends the session).
  - Gamepad / keyboard: Enhanced Input actions + contexts are **built in C++** (`CreateInventoryInput`, no
    assets): `GamepadIMC` (always, prio 0: View = toggle inventory, X = interact) and `InventoryIMC` (added
    while the inventory is open, `InventoryIMCPriority` = 1, consumes keys): D-pad / left stick / arrows =
    cursor, A / Enter = confirm, B = deselect or close, LB/RB = switch grid, X / R = take focused item,
    Y / F = take all. Keyboard Tab / E stay in the `IMC_Default` asset.
  - The cursor lives in `UInventoryUISubsystem` (`SetNavigableContainers` — the controller passes pawn
    inventory, then opened container; `MoveCursor` picks the hex neighbour best matching the direction and
    zig-zags vertically keeping a column; `SwitchCursorContainer`; `ConfirmAtCursor` selects / moves the
    selected item via `RequestTransferItem`). Mouse hover deactivates it; while active the info panel shows
    the item under it. Grids draw it via `UInventoryGridWidget::SyncSelection` → `SInventoryGrid::SetCursorCell`.
- **UI (MVP)**: Model interfaces (`IInventoryItem_Model`, `IItemsContainer_Model`, implemented by
  `UInventoryItem` / `UInventoryComponent`) → UObject presenters → view interfaces implemented by
  Slate widgets (`SInventoryGrid` on top of `SHhGridPanel`, `SInventoryGridItem`). UMG entry point:
  `UItemsContainer_Presenter::CreatePresenter(Model)` → `UInventoryGridWidget::AssignPresenter`, or
  `UInventoryGridWidget::bShowOwningPawnInventory` (follows the owning player's pawn).
  HUD (`BP_HUD`) is spawned by `BP_FirstPersonPlayerController`.
  - `SInventoryGrid` draws a background cell per grid cell (`CellBrush`, hex outline texture
    `M_UI_InventoryCell`) plus a round highlight fill (drop target > selected > cursor > hover), and handles all
    input itself: hover, click, drag & drop (`FInventoryItemDragDropOp`, payload = source presenter +
    slot). Cell under the cursor: `UHhGridFunctions::PointToHexAxialCoord`. Drops call
    `UItemsContainer_Presenter::HandleItemDropped` → source model `RequestTransferItem`.
  - Hover / selection are shared by all grids via `UInventoryUISubsystem` (local player subsystem):
    displayed item = hovered, else selected; items leaving a grid are forgotten. Clicking the selected
    item or an empty cell deselects.
  - `UInventoryItemInfoWidget` (UMG wrapper of `SInventoryItemInfo`, an `IInventoryItem_View`) shows
    the displayed item's icon / name / description; it lives in `BP_ItemInfo`.
- Custom content lives in `Content/Inventory` (note the folder typo `Bllueprints`).

## Current state (as of 2026-11-24)

C++ and all project blueprints compile. In PIE (standalone, server) both level containers fill
their inventories (`LogTypicalShooter: ... placed N of N manifest items`). The inventory screen has
been seen on screen (player grid, container grid, info panel, selection highlight). Multiplayer
replication (client → server RPCs over a real connection) is untested.

UI widgets: `BP_HUD` → `SetContainer(Container)` → `BP_ContainerGrid.SetContainer` →
`CreatePresenter` + `InventoryGridWidget_237.AssignPresenter`. `BP_InventoryGrid` holds `PlayerGrid`
(`UInventoryGridWidget`, `bShowOwningPawnInventory`). Both grids draw their own cells (the old
`BP_Slot_InventoryGrid` / `HhGridPanelWidget` background is gone; `BP_Slot_InventoryGrid` is unused).
`BP_ContainerGrid` buttons **Take** / **Take All** call `TakeSelectedItem` /
`TakeAllFromOpenedContainer` (keys: R / F, gamepad X / Y). `BP_ItemInfo` holds `ItemInfo`
(`UInventoryItemInfoWidget`).

The two level containers hold test items (`Test Medkit/Ammo/Grenade`, `Test Armor/Scope`) with
`FInfo_ItemFragment` (placeholder icon `T_UE_Logo_M`).

Inventory screen (Tab) and container opening (E) are wired and covered by the automation test
`TypicalShooter.Inventory.UIFlow` (`Source/TypicalShooter/Private/Tests/InventoryUIFlowTest.cpp`): it
starts PIE itself, feeds keys through `UGameViewportClient::InputKey` (Enhanced Input mappings are
exercised; OS → Slate focus is not) and checks Tab open/close, E at a container (grid shows every
item), transfer container → player (player grid updates), Take / Take All, server rejection for a far
container, the gamepad flow (X opens the container, D-pad cursor drawn in the grid, A select + move into the
container without jumping, RB, Y take all, B close, View toggle), W movement and jump. Gamepad keys go
through the same `InputKey` path. Real mouse drag & drop is covered only below the Slate event layer
(`UI.DragDrop*` call `MakeDragPayload` / `DropPayload` directly). Run via MCP: `AutomationTestToolset.DiscoverTests` →
`RunTests ["TypicalShooter.Inventory.UIFlow"]` → `GetTestResults` (the automation worker ends any
running PIE first). OS-level synthetic input (SendKeys / keybd_event) does not reach the editor's
game viewport — don't use it for testing. Logs: `LogTypicalShooter: Opened/Closed inventory|items container`; input setup
logs at Verbose. Before this, game input was effectively dead in PIE (the always-visible inventory
panel covered the screen and nothing set an input mode).

Open work:
- Multi-cell items (`FGrid_ItemFragment::Size` is ignored: one item per cell), stacking (`GetItemCount` is 1).
- Dropping items into the world.
- Gameplay effects of item adjacency on top of the layout event (design not started).
- Move the generated inventory input into IMC/IA assets (rebinding, on-screen button prompts).
- Multiplayer check of transfers and of the client-side layout event (dedicated server / client PIE).
- Leftover debug output outside the inventory: `BP_ItemsContainer` prints "Open / That's All" on interact.

Verify blueprints headlessly with:
`UnrealEditor-Cmd.exe <uproject> -run=CompileAllBlueprints -unattended -nopause -NullRHI -stdout`

Game design (roguelike arena loop, weapons, enemies) has not been started; `Variant_Shooter` template
code is the current basis for combat/AI.
