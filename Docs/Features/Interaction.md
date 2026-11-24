# Взаимодействие (E / геймпад X)

**Статус:** ✅ Работает для контейнеров предметов.

## Что это
Игрок жмёт **E** (на геймпаде **X**) рядом с интерактивным объектом — объект реагирует (сейчас единственный случай —
открыть контейнер).

## Как устроено
- [`UInteractionComponent`](../../Source/TypicalShooter/Public/InteractionComponent.h) на
  `ATypicalShooterCharacter`: каждый тик запускает асинхронный sphere overlap (`TraceDistance`, по умолчанию 100)
  по каналу `ECC_GameTraceChannel2` («PlayerInteraction») и хранит список акторов в радиусе (кроме владельца).
- Интерфейс `IPlayerInteractive` (`GetInteractionType`, `Interact`), BlueprintNativeEvent — можно
  реализовывать и в BP. `EInteractionType`: `None`, `Open`.
- `ATypicalShooterPlayerController::PrimaryInteract` (на `IA_Interact`):
  1. если контейнер уже открыт — закрывает инвентарь и контейнер;
  2. иначе берёт **ближайший** актор из overlap, реализующий `IPlayerInteractive`, и вызывает `Interact`;
  3. если это `AItemsContainer` — открывает экран инвентаря и контейнер ([InventoryUI.md](InventoryUI.md)).
- `PlayerTick` закрывает контейнер (но не инвентарь), если игрок вышел из радиуса или контейнер уничтожен.

## Настройка контента
Чтобы объект стал интерактивным: коллизия с объектным каналом «PlayerInteraction» (Overlap) +
реализация `IPlayerInteractive`.

## Тесты
`TypicalShooter.Inventory.UIFlow` — E и геймпадная X у контейнера открывают его.

## Ограничения и грабли
- Взаимодействие клиентское (UI); для действий, меняющих состояние мира, нужен будет server RPC.
- Подсказки «Нажмите E» нет.

## Открытые задачи
- [ ] Подсказка/подсветка цели взаимодействия.
- [ ] Другие типы взаимодействия (подбор предмета с земли и т.п.).

---
_Сверено с кодом: 2026-11-24_
