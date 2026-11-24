# Экран инвентаря и UI сетки

**Статус:** ✅ Экран открывается, предметы переносятся мышью (drag & drop) и геймпадом/клавиатурой (курсор),
внутри сетки и между инвентарём игрока и контейнером; Take / Take All есть на кнопках и клавишах.
Покрыто тестами, включая PIE-сценарий с геймпадом.

## Что это
Экран с сеткой инвентаря игрока, сеткой открытого контейнера и панелью описания предмета.

## Управление
| Действие | Клавиатура / мышь | Геймпад |
|---|---|---|
| Открыть/закрыть инвентарь | Tab | View (Back) |
| Взаимодействие (контейнер открывает инвентарь + контейнер; повторно — закрывает оба) | E | X |
| Перенести предмет | drag & drop мышью | A — взять предмет под курсором, A на другой клетке — положить (занято → обмен) |
| Двигать курсор | стрелки | D-pad / левый стик (удержание — автоповтор) |
| Подтвердить | Enter | A |
| Отменить выбор / закрыть инвентарь (если ничего не выбрано) | — | B |
| Переключить сетку (игрок ↔ контейнер) | — | LB / RB |
| Забрать предмет из контейнера | R (или кнопка Take) | X |
| Забрать всё | F (или кнопка Take All) | Y |

- Клавиши экрана инвентаря работают только пока он открыт и имеют приоритет над игровыми (A не прыгает, стик
  не двигает персонажа). Мышь и WASD при открытом инвентаре работают как раньше.
- Курсор «вверх/вниз» на гексах с острой вершиной идёт зигзагом между двумя верхними/нижними соседями,
  удерживая колонку. У каждой сетки курсор помнит свою клетку.
- Курсор виден с первого нажатия навигации и скрывается, когда мышь наводится на предметы. Пока курсор активен,
  панель описания показывает предмет под ним; «забрать» (R/X) берёт предмет под курсором, иначе выбранный.
- Esc в PIE завершает сессию — для UI не использовать.

## Как устроено
### Поток экрана
Всё в `ATypicalShooterPlayerController` (локально на клиенте):
- `OpenInventory` / `CloseInventory`: режим ввода (`GameAndUI` + курсор мыши / `GameOnly`), добавление/удаление
  контекста ввода инвентаря, BP-события `OnInventoryOpened/Closed` (показ `HUD.InventoryScreen`).
- `PrimaryInteract` у контейнера открывает инвентарь и контейнер (`OnItemsContainerOpened` → `HUD.SetContainer`);
  выход из радиуса закрывает только контейнер ([Interaction.md](Interaction.md)).
- После каждого открытия/закрытия контроллер сообщает подсистеме UI список навигируемых сеток
  (`UpdateNavigableContainers`: инвентарь пешки, затем контейнер).
- Лог: `LogTypicalShooter: Opened/Closed inventory|items container`.

### Ввод геймпада/клавиатуры
- Действия и контексты Enhanced Input **создаются в C++** (`CreateInventoryInput`, ассетов нет):
  `GamepadIMC` (всегда, приоритет 0: View, X) и `InventoryIMC` (пока открыт инвентарь, приоритет
  `InventoryIMCPriority` = 1). Клавиатурные Tab/E — в ассете `IMC_Default`.
- Логика курсора — в [`UInventoryUISubsystem`](../../Source/TypicalShooter/Public/UI/InventoryUISubsystem.h)
  (`MoveCursor`, `SwitchCursorContainer`, `ConfirmAtCursor`, `CancelSelection`, `GetFocusedItem`);
  перенос идёт тем же путём, что и drag & drop: `RequestTransferItem` модели источника.

### Архитектура UI (MVP)
```
Model (интерфейсы)                Presenter (UObject)              View (Slate)
IItemsContainer_Model  ──делегаты──▶ UItemsContainer_Presenter ──▶ SInventoryGrid (IItemsContainer_View)
  UInventoryComponent / AItemsContainer      │ создаёт по предмету            на SHhGridPanel
IInventoryItem_Model                  UInventoryItem_Presenter ──▶ SInventoryGridItem / SInventoryItemInfo
  UInventoryItem
```
- `UItemsContainer_Presenter::CreatePresenter(Model)` → `UInventoryGridWidget::AssignPresenter`, либо
  `UInventoryGridWidget::bShowOwningPawnInventory` (сетка следует за инвентарём пешки).
- `SInventoryGrid` рисует фоновые клетки и круглую подсветку (приоритет: цель drop → выбранный → курсор → наведение
  мыши), сам обрабатывает мышь: hover, click, drag & drop (`FInventoryItemDragDropOp`).
- Hover / выбор / курсор общие для всех сеток через `UInventoryUISubsystem` (local player subsystem);
  `UInventoryGridWidget::SyncSelection` переносит их на Slate-сетку.
- `UInventoryItemInfoWidget` (в `BP_ItemInfo`) показывает отображаемый предмет.
- Цепочка контейнера в BP: `BP_HUD.SetContainer` → `BP_ContainerGrid.SetContainer` → `CreatePresenter` +
  `AssignPresenter`. Кнопки Take / Take All в `BP_ContainerGrid`.

Код: [`Source/TypicalShooter/Public/UI/`](../../Source/TypicalShooter/Public/UI/),
[`TypicalShooterPlayerController`](../../Source/TypicalShooter/TypicalShooterPlayerController.h). Контент: `Content/Inventory`.

## Тесты
- `TypicalShooter.Inventory.UI.*` — презентер + Slate-сетка без окна: hover/click, drag & drop внутри и между
  сетками (ниже слоя событий Slate), панель описания, подсистема выбора.
- `TypicalShooter.Inventory.UI.Cursor.*` — навигация по гексам (зигзаг, края, диагонали), переключение сеток,
  A: выбор / перенос / обмен / между инвентарями, B, подсветка курсора
  ([InventoryCursorTest.cpp](../../Source/TypicalShooter/Private/Tests/InventoryCursorTest.cpp)).
- `TypicalShooter.Inventory.UIFlow` (PIE, [InventoryUIFlowTest.cpp](../../Source/TypicalShooter/Private/Tests/InventoryUIFlowTest.cpp)):
  клавиши подаются через `UGameViewportClient::InputKey` (Enhanced Input проверяется, фокус ОС → Slate — нет).
  Tab, E у контейнера, перенос, Take / Take All, отказ сервера для дальнего контейнера; геймпад: X, D-pad
  (курсор рисуется в сетке), A (выбор и перенос в контейнер, без прыжка), RB, Y, B, View; движение и прыжок.

## Ограничения и грабли
- Синтетический ввод ОС (SendKeys / keybd_event) до игрового вьюпорта редактора не доходит.
- Настоящий drag мышью проверен только ниже слоя событий Slate; клавиши R/F/Enter/стрелки автотестом не
  нажимаются (те же действия, что и у геймпада).
- Раскладка ввода инвентаря задана в коде, а не в ассетах: переназначение клавиш игроком пока невозможно.

## Открытые задачи
- [ ] Перенести контексты ввода инвентаря в ассеты (IMC/IA) и дать переназначение клавиш.
- [ ] Подсказки кнопок геймпада на экране.
- [ ] Переход курсора между сетками при выходе за край (сейчас только LB/RB).

---
_Сверено с кодом: 2026-11-24_
