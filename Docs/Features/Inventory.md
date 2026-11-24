# Инвентарь на гекс-сетке

**Статус:** 🟡 Ядро работает на сервере и покрыто тестами. У игрока инвентаря ещё нет (есть только у контейнеров),
клиентских запросов на перемещение (RPC) нет, репликация не проверялась в мультиплеере.

## Что это
Главная особенность игры: предметы раскладываются по клеткам **гексагональной** сетки
(осевые координаты `FHhAxialCoord` из плагина [HarhalHexagonalGrid](HexGrid.md)), а не по прямоугольной.

## Как устроено
- [`UInventoryComponent`](../../Source/TypicalShooter/Public/InventoryComponent.h) — компонент на акторе-владельце.
  Мутации `AddItem` (из манифеста или готового `UInventoryItem`), `RemoveItem` (по предмету или клетке),
  `MoveItem` — **только с авторитетом** (на сервере); на клиенте молча возвращают `nullptr`/`false`.
- [`FInventoryArray`](../../Source/TypicalShooter/Public/InventoryArray.h) — `FFastArraySerializer`
  записей `{Item, Location}`. Правила:
  - одна клетка — один предмет (`AddItem`/`MoveItem` в занятую клетку не проходят);
  - один и тот же предмет нельзя добавить дважды;
  - поиск линейный (инвентари маленькие).
- Компонент реализует `IItemsContainer_Model` и рассылает делегаты **added / removed / moved** —
  и на сервере (сразу при мутации), и на клиентах (из колбэков FastArray). Через них UI следит за изменениями.

### Репликация
- `UInventoryItem` реплицируется как subobject компонента (`ReadyForReplication` →
  `RegisterReplicatedSubObjects`).
- На клиенте запись может прийти раньше, чем разрешится объект предмета: флаг `bAddNotified` гарантирует,
  что «added» уйдёт один раз, когда предмет станет доступен. `LastKnownLocation` нужен, чтобы сообщить
  предыдущую клетку в «moved».
- В реальном мультиплеере не проверено.

## Тесты
`TypicalShooter.Inventory.Core.*` (мир без PIE):
`Add`, `AddToOccupiedCellFails`, `AddSameItemTwiceFails`, `MutationsNeedAuthority`, `Move`, `Remove`, …
([InventoryCoreTest.cpp](../../Source/TypicalShooter/Private/Tests/InventoryCoreTest.cpp)).

## Ограничения и грабли
- Размер сетки инвентаря нигде не ограничен на уровне данных — границы задаёт только UI.
- `FGrid_ItemFragment::Size` игнорируется (см. [Items.md](Items.md)).

## Открытые задачи
- [ ] `UInventoryComponent` на персонаже игрока (сейчас `BP_InventoryGrid` показывает пустые клетки).
- [ ] Server RPC для перемещения/переноса предметов, инициированных клиентом.
- [ ] Перенос предметов между инвентарями (контейнер ↔ игрок).
- [ ] Ограничение формы/размера сетки и многоклеточные предметы.
- [ ] Проверка репликации в listen/dedicated server.

---
_Сверено с кодом: 2026-11-24_
