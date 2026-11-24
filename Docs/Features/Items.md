# Предметы и фрагменты

**Статус:** ✅ Описание предмета через набор фрагментов работает; используется только `FInfo_ItemFragment` (UI).
`FGrid_ItemFragment::Size` пока нигде не учитывается — каждый предмет занимает одну клетку.

## Что это
Предмет описывается не классом, а **манифестом** — списком независимых фрагментов (данные-компоненты).
Новое свойство предмета = новый тип фрагмента, без наследования классов предметов.

## Как устроено
- [`FInventoryItemManifest`](../../Source/TypicalShooter/Public/InventoryItemManifest.h) — массив
  `TInstancedStruct<FItemFragment>`. Задаётся в редакторе (например, в `ItemsManifest` контейнера).
- [`FItemFragment`](../../Source/TypicalShooter/Public/ItemFragment.h) — базовый фрагмент с опциональным
  `FragmentTag` (GameplayTag), чтобы различать несколько фрагментов одного типа.
  - `FGrid_ItemFragment` — `Size` (размер на сетке; задел, не используется).
  - `FInfo_ItemFragment` — `DisplayName`, `Description`, `Icon` (данные для UI).
- Поиск фрагментов: `GetFragmentOfType<T>()` — первый фрагмент типа `T` (или наследника);
  `GetFragment<T>(Tag)` — первый с точным совпадением тега.
- `ManifestItem(Outer)` создаёт [`UInventoryItem`](../../Source/TypicalShooter/Public/InventoryItem.h) —
  runtime-экземпляр, хранящий копию манифеста (`FInstancedStruct`, реплицируется) и реализующий
  `IInventoryItem_Model` для UI. Без `FInfo_ItemFragment` имя/описание пустые, иконка — пустая кисть.
- `GetItemCount()` всегда `1`: стаков пока нет.

## Настройка контента
Добавить элемент в массив манифестов → в `Fragments` выбрать тип фрагмента и заполнить поля.
Тестовые предметы (`Test Medkit/Ammo/Grenade`, `Test Armor/Scope`) лежат в двух `BP_ItemsContainer`
на `Lvl_FirstPerson`, иконка-заглушка `T_UE_Logo_M`.

## Тесты
`TypicalShooter.Inventory.Core.ManifestFragments`, `TypicalShooter.Inventory.Core.ItemModel`
([InventoryCoreTest.cpp](../../Source/TypicalShooter/Private/Tests/InventoryCoreTest.cpp)).

## Ограничения и грабли
- `TInstancedStruct<FItemFragment>::Make(DerivedLvalue)` срезает до базового типа — писать
  `Make<FDerived>(Value)`.

## Открытые задачи
- [ ] Учитывать `FGrid_ItemFragment::Size` (многоклеточные предметы на гекс-сетке).
- [ ] Стаки / количество.
- [ ] Фрагменты с геймплейным эффектом (использование, экипировка, GAS).

---
_Сверено с кодом: 2026-11-24_
