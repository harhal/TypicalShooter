# Генерация лута: шаблоны предметов

**Статус:** 🟡 в работе (`feature/loot-generation`) — требования согласованы.

## Что это
Дизайнер описывает не конкретный предмет, а **шаблон** («какой бывает такой предмет»): тип в энергосети, возможные
формы выходов, диапазоны чисел, пул модификаторов, иконки и текст. Генератор по шаблону, **редкости** и **уровню**
бросает кубики и выдаёт обычный `FInventoryItemManifest`. Контейнер на уровне ссылается на **лут-таблицу** и при
старте наполняется случайными предметами. Чем выше уровень — тем больше числа; чем выше редкость — тем больше и
сильнее модификаторы (шаги порогов энергосети).

## Требования
Ассеты (все — `UPrimaryDataAsset`, лежат в `Content/Inventory/Loot`):

- R1. **Шаблон предмета** (`ULootItemTemplate`) задаёт: жёсткий тип в энергосети (`EPowerItemType`) и его неслучайные
  поля (`AmmoType`, `Ability` приёмника); взвешенный список форм выходов (масок `OpenFaces`) и флаг случайного
  начального поворота; диапазоны чисел энергосети (источник: `Power`, `Capacity`, `CostPerUse`, `RechargeDelay`;
  приёмник: `RequiredPower`, `BaseStats`); взвешенный пул модификаторов; имя, шаблон описания и иконку для каждой
  редкости; список прочих фиксированных фрагментов, копируемых как есть.
- R2. **Диапазон** числа = `Min`/`Max` + флаг «масштабируется от уровня». Масштабируемое значение умножается на
  общий множитель уровня — кривую в настройках проекта (`ULootSettings`, Project Settings → Game → Loot);
  уровень ограничен 1…100; целые округляются, результат не меньше 0. Кривая по умолчанию линейная:
  ×1 на уровне 1, ×5.95 на уровне 100 (+5 % за уровень).
- R3. **Модификатор** (`ULootModifier`) = один шаг порога: тир (int), тег, диапазон `Threshold`, диапазоны бонусов
  атаки (`AttackBonuses`: атрибут + диапазон), опциональный `CharacterEffect` GE (применяется с уровнем предмета), строка описания с
  плейсхолдерами. Даёт ровно один `FPowerThresholdEffect`.
- R4. **Редкость** (`ULootRarity`): имя, цвет, массив слотов модификаторов `{Tier, Tag}`. Для каждого слота
  генератор выбирает из пула шаблона модификатор с тегом, совпадающим с тегом слота (иерархически, `MatchesTag`),
  и **тиром строго равным** тиру слота; выбор по весам пула, один модификатор не выпадает дважды на предмете.
  Нет кандидата — слот пропускается (Warning в лог). Шаги в итоговом `FPowerEffects_ItemFragment` отсортированы
  по `Threshold`.
- R5. **Лут-таблица** (`ULootTable`): диапазон количества предметов, взвешенный список шаблонов (повторы разрешены),
  список редкостей с весом-кривой от уровня. Бросок таблицы для уровня L даёт N манифестов.
- R6. Генерация детерминирована: всё случайное идёт через `FRandomStream`; одинаковые (ассеты, уровень, seed) →
  одинаковые манифесты.
- R7. **Текст**: имя берётся из шаблона; описание = `FText::Format` шаблона описания с выпавшими значениями
  (`{Power}`, `{Capacity}`, `{CostPerUse}`, `{RechargeDelay}`, `{RequiredPower}`, `{Level}`, `{Rarity}`), ниже —
  строки модификаторов (`{Threshold}`, `{Bonus}` …). Иконка — из шаблона по редкости. Всё попадает в обычный
  `FInfo_ItemFragment`.
- R8. Сгенерированный предмет несёт **фрагмент происхождения** (`FLootOrigin_ItemFragment`): шаблон, редкость,
  уровень, выпавшие модификаторы, seed. Ручные предметы его не имеют.
- R9. **Контейнер**: у `AItemsContainer` есть `LootTable`, `LootLevel` и `LootSeed` (0 = случайный). На сервере в
  `BeginPlay` после ручного `ItemsManifest` (он остаётся) кладутся предметы из броска таблицы, по той же гекс-спирали;
  не поместившиеся — Warning в лог. Без таблицы поведение прежнее.
- R10. **UI — редкость** (из фрагмента происхождения): в сетке — рамка ячейки цветом редкости; в инфо-панели —
  имя цветом редкости и строка с названием редкости. У ручных предметов — как сейчас.
- R11. **UI — подложка**: ячейка предмета рисует **процедурную подложку с выходами** по маске `OpenFaces` с учётом
  `Rotation` (одинаковая для всех предметов с одинаковой схемой граней, отдельных текстур нет); иконка — поверх,
  по центру, не вращается. Относится ко всем предметам с `FPowerGrid_ItemFragment`, не только к сгенерированным.
  Подложка **заменяет** текущую отрисовку граней предмета в `InventoryGridPowerPaint.cpp` (связи, подписи мощности /
  обоймы и метки слотов остаются).

**Вне рамок:** дроп с врагов и награды за волну (лут-таблица к этому готова, подключение — отдельная фича);
источник уровня из прогресса забега (пока свойство экземпляра контейнера); модификаторы, меняющие числа энергосети
(только шаги порога); случайный тип предмета; префиксы/суффиксы в имени; многоклеточные предметы; торговля/крафт;
проверка в мультиплеере (генерация только на сервере, предметы реплицируются как обычно).

**Открытые вопросы:** нет.

## План итераций
- [x] 1. `FLootIntRange` / `FLootFloatRange` + `ULootSettings` (кривая множителя) + бросок с масштабированием —
  тест: `TypicalShooter.Loot.Range.*`
- [x] 2. `ULootItemTemplate` (тип, фиксированные фрагменты, формы с весами, поворот) + `LootGenerator::Generate`
  + `FLootOrigin_ItemFragment`, детерминизм по seed — тест: `TypicalShooter.Loot.Generator.Shape`,
  `.Determinism`
- [x] 3. Числа энергосети источника / приёмника из диапазонов с уровнем — тест: `TypicalShooter.Loot.Generator.PowerParams`
- [x] 4. `ULootRarity` + `ULootModifier`: выбор по слотам (тег, тир строго, веса, без повторов, пропуск без
  кандидата) → `FPowerEffects_ItemFragment` — тест: `TypicalShooter.Loot.Modifiers.*`, `.Generator.Modifiers`
- [x] 4б. Уровень GE модификатора = уровень предмета: поле уровня в `FPowerThresholdEffect`, его учитывает
  `UPowerEffectsComponent` (затрагивает энергосеть, поэтому отдельный шаг) — тест: `TypicalShooter.PowerGrid.Effects.*`,
  `TypicalShooter.Loot.Generator.Modifiers`
- [x] 5. `FInfo_ItemFragment`: имя, описание с плейсхолдерами + строки модификаторов, иконка по редкости —
  тест: `TypicalShooter.Loot.Generator.Info`
- [x] 6. `ULootTable`: количество, шаблоны по весам, редкость по весам-кривым от уровня — тест:
  `TypicalShooter.Loot.Table.*`
- [x] 7. `AItemsContainer`: `LootTable` / `LootLevel` / `LootSeed`, заполнение после ручного манифеста — тест:
  `TypicalShooter.Loot.Container.Fill`
- [x] 8. UI: цвет редкости в сетке и инфо-панели — тест: `TypicalShooter.Loot.UI.Rarity*`
- [x] 9. UI: процедурная подложка с выходами + иконка по центру — тест: `TypicalShooter.Loot.UI.Underlay`
  (+ скриншот из `PowerGrid.Flow` для визуальной проверки)
- [x] 10. Контент: 2–3 редкости, несколько модификаторов, шаблоны источника / коннектора / приёмника, лут-таблица;
  подключить к одному контейнеру на `Lvl_FirstPerson` — ручная проверка в PIE + `TypicalShooter.Inventory.UIFlow`

## Как устроено
Код — в `Source/TypicalShooter/Public|Private/Loot/`.
- [`LootRange.h`](../../Source/TypicalShooter/Public/Loot/LootRange.h): `FLootIntRange` / `FLootFloatRange`
  (`Min`, `Max`, `bScalesWithLevel`). `Roll(Stream, LevelMultiplier)` — равномерно в [Min, Max] (порядок границ не
  важен), при `bScalesWithLevel` умножается на множитель (целые округляются), не меньше 0. Все броски — через
  переданный `FRandomStream`.
- [`LootSettings.h`](../../Source/TypicalShooter/Public/Loot/LootSettings.h): `ULootSettings` (`UDeveloperSettings`,
  `DefaultGame.ini`): кривая `LevelMultiplier` (по умолчанию линейная ×1 → ×5.95), `ClampLevel` (1…100),
  `GetLevelMultiplier(Level)` (пустая кривая = 1).
- [`LootRandom.h`](../../Source/TypicalShooter/Public/Loot/LootRandom.h): `PickWeightedIndex` — выбор по весам
  (веса ≤ 0 не выигрывают), общий для форм, пулов и таблиц.
- [`LootItemTemplate.h`](../../Source/TypicalShooter/Public/Loot/LootItemTemplate.h): `ULootItemTemplate` —
  `ItemType`, `FaceShapes` (`FLootFaceShape`: маска + вес; пусто = все грани), `bRandomRotation`, `FixedFragments`,
  `Source` / `Receiver` (видны по `ItemType`), `ModifierPool` (`FLootModifierEntry`: модификатор + вес),
  `DisplayName`, `DescriptionFormat`, `IconsByRarity` + `DefaultIcon`.
- [`LootItemText.h`](../../Source/TypicalShooter/Public/Loot/LootItemText.h): `LootItemText::MakeInfo` собирает
  `FInfo_ItemFragment`: имя шаблона; описание = `DescriptionFormat` с `{Power}`, `{Capacity}`, `{CostPerUse}`,
  `{RechargeDelay}`, `{RequiredPower}`, `{Level}`, `{Rarity}` + по строке `DescriptionLine` на модификатор
  (`{Threshold}`, `{Bonus0}`, `{Bonus1}` …) в порядке порогов, через `\n`; дробные — до 2 знаков; иконка — по
  редкости, иначе `DefaultIcon`.
- [`LootModifier.h`](../../Source/TypicalShooter/Public/Loot/LootModifier.h): `ULootModifier` — `Tier`, `Tag`
  (`Loot.Modifier.*`, нативные теги в `LootTags.h`), диапазон `Threshold`, `AttackBonuses`, `CharacterEffect`,
  `DescriptionLine`; `Roll` → один `FPowerThresholdEffect`.
- [`LootRarity.h`](../../Source/TypicalShooter/Public/Loot/LootRarity.h): `ULootRarity` — `DisplayName`, `Color`,
  `Slots` (`FLootModifierSlot`: `Tier`, `Tag`; пустой тег = любой).
- [`LootModifierPicker.h`](../../Source/TypicalShooter/Public/Loot/LootModifierPicker.h): по слоту — кандидаты пула с
  тиром == тиру слота и тегом, совпадающим с тегом слота (`MatchesTag`, иерархически), ещё не взятые; выбор по весам.
  Нет кандидата — Warning `Loot: rarity ... has no modifier for slot`, слот пропускается.
- [`LootPowerParams.h`](../../Source/TypicalShooter/Public/Loot/LootPowerParams.h): `FLootSourceParams` (диапазоны
  `Power`, `Capacity`, `CostPerUse`, `RechargeDelay`, фиксированный `AmmoType`), `FLootReceiverParams` (фиксированный
  `Ability`, диапазон `RequiredPower`, `BaseStats` из `FLootAttackStatRange`). Каждый `Roll` даёт готовые
  `FPowerSourceParams` / `FPowerReceiverParams`; коннектор оставляет их по умолчанию.
- [`LootGenerator.h`](../../Source/TypicalShooter/Public/Loot/LootGenerator.h): `LootGenerator::Generate(Template,
  {Level, Seed, Rarity})` → `FInventoryItemManifest` = фиксированные фрагменты + `FInfo_ItemFragment` +
  `FPowerGrid_ItemFragment` +
  `FPowerEffects_ItemFragment` (только если выпали модификаторы; шаги отсортированы по `Threshold`, у каждого
  `CharacterEffectLevel` = уровень предмета — с ним `UPowerEffectsComponent` применяет GE) + фрагмент
  происхождения. Весь рандом — из `FRandomStream(Seed)`. **Случайный поворот** применяется к самой маске
  (`HexFaces::RotateMask`), а не к `UInventoryItem::Rotation`: поворот — состояние экземпляра, в манифесте его нет.
- [`LootOriginFragment.h`](../../Source/TypicalShooter/Public/Loot/LootOriginFragment.h): `FLootOrigin_ItemFragment`
  — шаблон, уровень (после clamp), seed, редкость, модификаторы в порядке слотов.
- [`LootTable.h`](../../Source/TypicalShooter/Public/Loot/LootTable.h): `ULootTable` — `ItemCount` (диапазон, может
  масштабироваться от уровня), `Templates` (`FLootTemplateEntry`: шаблон + вес), `Rarities` (`FLootRarityEntry`:
  редкость + кривая веса от уровня; пустая кривая = вес 1; пустой список = предметы без редкости).
  `Roll(Level, Seed)`: на каждый предмет — шаблон и редкость по весам, свой seed из потока таблицы → `Generate`.
  Нет ни одного шаблона с весом — пустой результат.
- [`AItemsContainer`](../../Source/TypicalShooter/Public/ItemsContainer.h): `LootTable`, `LootLevel`, `LootSeed`
  (0 = случайный). `BeginPlay` (сервер): `PlaceManifestItems` (ручные, как раньше), затем `PlaceLootItems` (бросок
  таблицы в первые свободные клетки). Лог: `placed N loot items from <таблица> (level L, seed S)`; переполнение —
  Warning `did not fit`.
- UI редкости: `IInventoryItem_Model::GetItemRarity` (`UInventoryItem` читает фрагмент происхождения) →
  `IInventoryItem_View::SetRarity` (`UI/InventoryItem/ItemRarityDisplay.h`): `SInventoryGridItem` — гекс-рамка цветом
  редкости (`GetFrameColor`), `SInventoryItemInfo` — имя цветом редкости + строка с её названием.
- Подложка с выходами: `SInventoryGrid::ApplyUnderlays` (`InventoryGridUnderlay.cpp`; на `SetPowerGrid`, добавление и
  перенос предмета) передаёт в `SInventoryGridItem::SetUnderlay` грани клетки из `FPowerGridDisplay` (уже с поворотом)
  и `bPowered`. `SInventoryGridItem::OnPaint` рисует «спицы» из центра к каждой открытой грани
  (`PowerGridOverlay::GetFaceSpoke`, жёлтые / серые), поверх — рамку редкости и детей (иконка по центру, имя внизу).
  Штрихи граней в `InventoryGridPowerPaint.cpp` убраны; связи, подписи и метки слотов рисуются как раньше.

## Настройка контента
- Project Settings → Game → Loot: кривая множителя уровня.
- Data Assets классов `LootItemTemplate`, `LootModifier`, `LootRarity`, `LootTable`.
- Экземпляр `BP_ItemsContainer` на уровне: категория Loot — `LootTable`, `LootLevel`, `LootSeed`.
- Тестовый контент в `Content/Inventory/Loot` (иконки — заглушка `T_UE_Logo_M`):
  - редкости `DA_Rarity_Common` (серая, слот тир 1), `DA_Rarity_Rare` (синяя, 2 слота тир 1), `DA_Rarity_Epic`
    (фиолетовая, тир 2 + тир 1); все слоты — `Loot.Modifier.Attack`;
  - модификаторы `DA_Mod_Physical_T1`, `DA_Mod_Acid_T1`, `DA_Mod_Speed_T1`, `DA_Mod_Physical_T2`, `DA_Mod_Electric_T2`
    (бонусы атаки, урон масштабируется от уровня);
  - шаблоны `DA_Loot_ParticleCore` (источник на `Ammo.Particle`, все грани или тройник), `DA_Loot_Conduit`
    (коннектор: прямой / изогнутый / угол / тройник, без модификаторов), `DA_Loot_Amplifier` (коннектор с пулом
    модификаторов), `DA_Loot_RifleReceiver` (приёмник основной атаки, урон от уровня, пул модификаторов);
  - таблица `DA_LootTable_TestCrate`: 3–5 предметов, Conduit 3 : Amplifier 2 : Core 1 : Rifle 1; веса редкостей от
    уровня: Common 6→1, Rare 2→4, Epic 0.5→3.
- На `Lvl_FirstPerson` первый контейнер (3 ручных тестовых предмета) получил `DA_LootTable_TestCrate`, уровень 10,
  `LootSeed = 0`: при каждом запуске PIE в нём новые 3–5 случайных предметов.

## Тесты
- `TypicalShooter.Loot.Range.*` (`Private/Tests/LootRangeTest.cpp`, без мира): границы, масштабирование и округление,
  неотрицательность, детерминизм по seed, множитель уровня и clamp.
- `TypicalShooter.Loot.Random.PickWeighted`, `TypicalShooter.Loot.Generator.*` (`LootGeneratorTest.cpp`, общие
  хелперы — `LootTestFixture.h`): форма по весам и поворот, фиксированные фрагменты, детерминизм, происхождение.
- `TypicalShooter.Loot.Generator.PowerParams.*` (`LootPowerParamsTest.cpp`): числа источника / приёмника с уровнем,
  коннектор не трогает их.
- `TypicalShooter.Loot.Modifiers.*`, `TypicalShooter.Loot.Generator.Modifiers` (`LootModifiersTest.cpp`): тег / тир
  слота, без повторов, пропуск слота, бросок шага, сортировка, происхождение.
- `TypicalShooter.Loot.Generator.Info.*` (`LootInfoTest.cpp`): плейсхолдеры, строки модификаторов, иконка по редкости.
- `TypicalShooter.Loot.Table.*` (`LootTableTest.cpp`): количество, детерминизм и seed на предмет, веса шаблонов,
  веса редкостей от уровня.
- `TypicalShooter.Loot.Container.*` (`LootContainerTest.cpp`, голый мир с `BeginPlay`): ручные + табличные предметы,
  детерминизм по seed, без таблицы, переполнение сетки, seed 0.
- `TypicalShooter.Loot.UI.Rarity*` (`LootRarityUITest.cpp`, презентеры + Slate без окна): модель, рамка в сетке,
  инфо-панель.
- `TypicalShooter.Loot.UI.Underlay.*` (`LootUnderlayTest.cpp`): геометрия спиц, сетка раздаёт грани виджетам
  (и снимает, и сразу выдаёт новым). Визуально — скриншот `TypicalShooter.PowerGrid.Flow` в рендерящем редакторе
  (`Saved/Screenshots/WindowsEditor/PowerGridFlow.png`; работает и из `UnrealEditor-Cmd` без `-NullRHI`).

## Ограничения и грабли
- В тестах держите сгенерированный манифест в локальной переменной: указатель на фрагмент временного манифеста
  висит (у хелперов `PowerGridOf` / `OriginOf` перегрузка для rvalue удалена).
- Не кладите `FInfo_ItemFragment` в `FixedFragments` шаблона: инфо генерируется, а `GetFragmentOfType` вернёт первый
  (фиксированный).

## Открытые задачи
- [ ] Дроп с врагов / награда за волну через `ULootTable`.
- [ ] Уровень лута из прогресса забега.

---
_Сверено с кодом: 2026-11-26_
