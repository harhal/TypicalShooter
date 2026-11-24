# Экран инвентаря (Tab) и UI сетки

**Статус:** 🟡 Открытие/закрытие экрана и отображение предметов контейнера работают и покрыты PIE-тестом.
Drag & drop — заглушки; инвентарь игрока показывает только пустые клетки.

## Что это
**Tab** — открыть/закрыть экран инвентаря (курсор, ввод game+UI). **E** у контейнера — открыть инвентарь
вместе с сеткой контейнера. Предметы рисуются иконкой и именем в гекс-клетках.

## Управление и поток экрана
Всё в `ATypicalShooterPlayerController` (локально на клиенте):
- `IA_ToggleInventory` (Tab) → `OpenInventory` / `CloseInventory`. Открыт — `GameAndUI` + курсор;
  закрыт — `GameOnly` (так же в `BeginPlay`, что отдаёт фокус вьюпорту).
- Закрытие инвентаря закрывает и контейнер; E при открытом контейнере закрывает оба;
  выход из радиуса закрывает только контейнер ([Interaction.md](Interaction.md)).
- BP-события в `BP_FirstPersonPlayerController`: `OnInventoryOpened/Closed` показывают/прячут
  `HUD.InventoryScreen`; `OnItemsContainerOpened/Closed` → `HUD.SetContainer` + показ/скрытие `BP_ContainerGrid`.
  Кнопка Close в `BP_InventoryGrid` вызывает `CloseInventory`.
- Лог: `LogTypicalShooter: Opened/Closed inventory|items container`.

## Архитектура UI (MVP)
```
Model (интерфейсы)                Presenter (UObject)              View (Slate)
IItemsContainer_Model  ──делегаты──▶ UItemsContainer_Presenter ──▶ SInventoryGrid (IItemsContainer_View)
  UInventoryComponent / AItemsContainer      │ создаёт по предмету            на SHhGridPanel
IInventoryItem_Model                  UInventoryItem_Presenter ──▶ SInventoryGridItem (IInventoryItem_View)
  UInventoryItem
```
- `UItemsContainer_Presenter::CreatePresenter(Model)` → `UInventoryGridWidget::AssignPresenter` (UMG-обёртка
  над `SInventoryGrid`, держит ссылку на презентер). Презентер делает начальное заполнение и дальше
  синхронизирует add/remove/move; предметы во view адресуются по `FGuid` слота.
- Цепочка в BP: `BP_HUD.SetContainer` → `BP_ContainerGrid.SetContainer` → `CreatePresenter` + `AssignPresenter`.
- `BP_ContainerGrid` / `BP_InventoryGrid` рисуют фоновые клетки (`BP_Slot_InventoryGrid`) в
  `HhGridPanelWidget` `GridPanel`; в `BP_ContainerGrid` сетка предметов лежит поверх в Overlay.
- `BP_ItemInfo` есть на экране, но к выбору предмета ещё не привязан.

Код: [`Source/TypicalShooter/Public/UI/`](../../Source/TypicalShooter/Public/UI/). Контент: `Content/Inventory`.

## Тесты
`TypicalShooter.Inventory.UIFlow`
([InventoryUIFlowTest.cpp](../../Source/TypicalShooter/Private/Tests/InventoryUIFlowTest.cpp)): сам запускает PIE,
подаёт клавиши через `UGameViewportClient::InputKey` (Enhanced Input проверяется, фокус ОС → Slate — нет) и проверяет
Tab, E у контейнера (видны все предметы), движение W и прыжок.

## Ограничения и грабли
- Esc в PIE завершает сессию — для UI не использовать.
- Синтетический ввод ОС (SendKeys / keybd_event) до игрового вьюпорта редактора не доходит.
- `SInventoryGridItem`: `HandleHovered/HandleFocus/HandleDrag` — пустые заглушки.

## Открытые задачи
- [ ] Drag & drop между клетками и между сетками (+ server RPC, см. [Inventory.md](Inventory.md)).
- [ ] Показ описания выбранного/наведённого предмета в `BP_ItemInfo`.
- [ ] Сетка инвентаря игрока с реальными предметами.
- [ ] Проверить UI вживую на экране (пока подтверждён только автотестом).

---
_Сверено с кодом: 2026-11-24_
