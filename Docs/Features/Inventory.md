# Инвентарь на гекс-сетке

**Статус:** 🟡 Ядро, перенос предметов (внутри инвентаря и между инвентарями) и события раскладки работают
на сервере и покрыты тестами; у игрока есть свой инвентарь. Репликация в реальном мультиплеере не проверялась.

## Что это
Главная особенность игры: предметы раскладываются по клеткам **гексагональной** сетки
(осевые координаты `FHhAxialCoord` из плагина [HarhalHexagonalGrid](HexGrid.md)), а не по прямоугольной.
Взаимное расположение предметов (соседство) в дальнейшем будет влиять на геймплей, поэтому изменения
раскладки сообщаются отдельным событием с готовым списком изменившихся соседств.

## Как устроено
- [`UInventoryComponent`](../../Source/TypicalShooter/Public/InventoryComponent.h) — компонент на акторе-владельце
  (у персонажа игрока и у контейнеров). Сетка — гекс радиуса `GridRadius` (по умолчанию 2 = 19 клеток);
  `IsLocationInBounds`, `FindFreeLocation` (по спирали от центра).
- Мутации — **только с авторитетом** (на сервере); на клиенте молча возвращают `nullptr`/`false`:
  - `AddItem` (из манифеста или готового `UInventoryItem`), `RemoveItem` (по предмету или клетке), `MoveItem`;
  - `TransferItem(Item, Target, Location)` — перенос внутри инвентаря или в другой; занятая клетка → **обмен**
    местами. Между инвентарями предмет пересоздаётся из манифеста (иного состояния у предметов пока нет);
  - `TransferItemToFreeLocation` — в первую свободную клетку цели.
- [`FInventoryArray`](../../Source/TypicalShooter/Public/InventoryArray.h) — `FFastArraySerializer` записей
  `{Item, Location}`: одна клетка — один предмет, один предмет — одна запись, поиск линейный.
  Копирование массива намеренно не переносит `Owner` (BP-акторы копируют компонент из CDO).

### Делегаты
1. **Поштучные** `added / removed / moved` (интерфейс `IItemsContainer_Model`) — на сервере сразу при каждой
   мутации, на клиентах из колбэков FastArray. Их использует UI (презентер сетки). Могут наблюдать
   промежуточное состояние (например, середину обмена между инвентарями).
2. **Событие раскладки** `GetLayoutChangedDelegate()` (C++) / `OnLayoutChanged` (Blueprint) —
   [`FInventoryLayoutChange`](../../Source/TypicalShooter/Public/InventoryLayout.h), **одно на атомарную операцию**,
   когда инвентарь уже согласован:
   - `Placements` — для каждого предмета, чьё место изменилось: было/стало (добавлен, удалён, перемещён);
   - `Neighbors` — для каждого предмета, у которого изменился набор соседей (гекс-расстояние 1):
     `GainedNeighbors` / `LostNeighbors`. Пара соседей сообщается с обеих сторон; добавленный предмет
     «получает» всех соседей, удалённый — «теряет».
   - Сервер: каждая мутация открывает `FScopedLayoutBatch`; обмен/перенос — одно событие на каждый из двух
     инвентарей. Многошаговые операции оборачиваются явно (Take All в `ServerTakeItems`).
   - Клиент: событие после каждого приёма репликации (`FInventoryArray::PostReplicatedReceive`).
   - Реализация: компонент помнит последнюю сообщённую раскладку и считает разницу
     (`InventoryLayout::Diff`) — поэтому событие корректно при любом порядке внутренних шагов.
- Запросы соседства: `GetNeighborItems(Location)`, `GetItemNeighbors(Item)`;
  гекс-направления — `InventoryLayout::GetNeighborOffsets / GetNeighborLocations`.

### Запросы клиента
UI вызывает `IItemsContainer_Model::RequestTransferItem` → `UInventoryComponent` передаёт его первому локальному
`ATypicalShooterPlayerController::RequestTransferItem` → RPC `ServerTransferItem` (проверка `CanAccessInventory`:
свой инвентарь или контейнер в радиусе доступа). Take / Take All → `ServerTakeItems` (в первые свободные клетки
инвентаря пешки). Без контроллера (тесты, инструменты) запрос применяется напрямую при наличии авторитета.

### Репликация
- `UInventoryItem` реплицируется как subobject компонента (`ReadyForReplication` → `RegisterReplicatedSubObjects`).
- На клиенте запись может прийти раньше, чем разрешится объект предмета: флаг `bAddNotified` гарантирует,
  что «added» уйдёт один раз, когда предмет станет доступен. `LastKnownLocation` нужен, чтобы сообщить
  предыдущую клетку в «moved».
- В реальном мультиплеере не проверено.

## Тесты
- `TypicalShooter.Inventory.Core.*` (мир без PIE): добавление/удаление/перемещение, границы, свободная клетка,
  перенос внутри и между инвентарями, авторитет, `OwnerSurvivesArchetypeCopy`
  ([InventoryCoreTest.cpp](../../Source/TypicalShooter/Private/Tests/InventoryCoreTest.cpp),
  [InventoryTransferTest.cpp](../../Source/TypicalShooter/Private/Tests/InventoryTransferTest.cpp)).
- `TypicalShooter.Inventory.Layout.*`: `Diff`, `NeighborLocations`, `NeighborQueries`, `MoveBroadcastsOnce`,
  `SwapIsOneEvent`, `TransferBetweenInventories` (по событию на инвентарь, оба согласованы), `ScopedBatch`
  ([InventoryLayoutTest.cpp](../../Source/TypicalShooter/Private/Tests/InventoryLayoutTest.cpp)).

## Ограничения и грабли
- `FGrid_ItemFragment::Size` игнорируется (см. [Items.md](Items.md)) — каждый предмет занимает одну клетку.
- При переносе между инвентарями предмет — **новый объект**: в событии раскладки это «удалён» в источнике
  и «добавлен» в цели, а не «перемещён».
- Клиентская ветка события раскладки (`PostReplicatedReceive`) тестами не покрыта.

## Открытые задачи
- [ ] Многоклеточные предметы (`Size`) и стаки (`GetItemCount` = 1).
- [ ] Эффекты соседства предметов поверх события раскладки (геймдизайн).
- [ ] Выбрасывание предметов в мир.
- [ ] Проверка репликации в listen/dedicated server (включая событие раскладки на клиенте).

---
_Сверено с кодом: 2026-11-24_
