# Гекс-сетка (плагин HarhalHexagonalGrid)

**Статус:** ✅ Математика сетки и Slate/UMG-панель работают; покрыто тестами `HarhalHexagonalGrid.*`.

## Что это
Собственная библиотека автора — отдельный git-сабмодуль `Plugins/HarhalHexagonalGrid`
([GitHub](https://github.com/harhal/HarhalHexagonalGrid)). Не зависит от кода игры.
Подробная документация плагина — в его `README.md`; здесь только то, как игра им пользуется.

## Что игра использует
- `FHhAxialCoord` — осевые координаты клетки; ключ расположения предмета в [инвентаре](Inventory.md).
- `UHhGridFunctions::MakeHexAxialCoordFromSpiral(n)` — n-я клетка спирали от центра; раскладка
  предметов в [контейнерах](ItemsContainer.md). Также `GetHexSpiralLength(r)`, `HexAxialCoordToPoint`.
- `SHhGridPanel` / `UHhGridPanelWidget` — панель, раскладывающая детей по гексам: основа `SInventoryGrid`
  и фоновых клеток в `BP_ContainerGrid` / `BP_InventoryGrid` ([InventoryUI.md](InventoryUI.md)).

## Как менять
Изменения плагина — два коммита: сначала внутри сабмодуля (и push), затем указатель сабмодуля здесь
(см. раздел Git в `CLAUDE.md`).

---
_Сверено с кодом: 2026-11-24_
