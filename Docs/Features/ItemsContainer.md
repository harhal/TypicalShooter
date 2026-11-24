# Контейнеры предметов в мире

**Статус:** ✅ Контейнеры на уровне заполняются на сервере и открываются по E.

## Что это
Актор в мире (ящик, тайник), в котором лежат предметы. Подходишь, жмёшь E — открывается его сетка рядом
с инвентарём игрока.

## Как устроено
- [`AItemsContainer`](../../Source/TypicalShooter/Public/ItemsContainer.h):
  - `SphereCollision` — объект канала `ECC_GameTraceChannel2` («PlayerInteraction»), отвечает Overlap
    только на этот канал; по ней его находит [взаимодействие](Interaction.md).
  - `InventoryComponent` — хранит предметы ([Inventory.md](Inventory.md)).
  - `ItemsManifest` — массив `FInventoryItemManifest` (как `FInstancedStruct`), задаётся в редакторе.
- В `BeginPlay` на сервере каждый непустой манифест превращается в предмет и кладётся по гекс-спирали
  (`UHhGridFunctions::MakeHexAxialCoordFromSpiral(i)`: центр, затем кольца). Пустые элементы массива пропускаются.
  Лог: `LogTypicalShooter: <имя>: placed N of M manifest items`.
- Реализует `IPlayerInteractive` (тип взаимодействия `Open`) и `IItemsContainer_Model`, пробрасывая
  модель в свой `InventoryComponent` — поэтому UI может принимать сам контейнер как модель.

## Настройка контента
`Content/Inventory/Bllueprints/BP_ItemsContainer` (опечатка в имени папки историческая).
Два экземпляра на `Lvl_FirstPerson` с тестовыми предметами. Радиус сферы задаётся в экземпляре/BP.

## Тесты
Косвенно — `TypicalShooter.Inventory.UIFlow` (открытие контейнера, в сетке видны все предметы).

## Открытые задачи
- [ ] Забирать/класть предметы (зависит от drag & drop и RPC).
- [ ] Лут-таблицы / случайное наполнение вместо фиксированного манифеста.

---
_Сверено с кодом: 2026-11-24_
