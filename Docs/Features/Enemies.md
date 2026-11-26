# Враги: стрелки, волны, ИИ на StateTree

**Статус:** ⚪ В разработке (ветка `feature/enemies`): требования и план согласуются.

## Что это
Первые враги арены. Враг — персонаж с GAS-здоровьем и той же атакой снарядами, что у игрока, только слабее.
Он держится на расстоянии от игрока и стреляет, когда видит его. Враги приходят волнами из спавнера: следующая
волна — после гибели всей предыдущей. Поведение — StateTree; все живые враги зарегистрированы в «директоре»
мира — точке будущей координации (кто стреляет, кто куда встаёт).

## Требования

**Здоровье игрока**
- R1. У игрока GAS-здоровье (`UHealthAttributeSet`) и GAS-получатель урона: снаряды врагов его ранят.
- R2. Игрок пока бессмертен: при нуле здоровье сразу восстанавливается до максимума (как у манекена), в лог
  пишется «поражение». Настоящая смерть — отдельная фича.
- R3. HUD показывает здоровье игрока «Health: текущее / максимум».

**Враг**
- R4. `AEnemyCharacter` (ACharacter): свой ASC, здоровье, GAS-получатель урона, набор атрибутов атаки, дуло
  `Muzzle`. Снаряды игрока его ранят.
- R5. Атака врага — та же `UAttackGameplayAbility` (основной слот) со снарядами; базовые параметры задаются
  в классе врага (`AttackStats`: урон, скорость снаряда, очередь, интервалы) и по умолчанию слабее игрока. Боеприпасы
  и энергосеть врагу не нужны.
- R6. При нуле здоровья враг умирает: перестаёт стрелять и двигаться, сообщает о смерти (делегат / BP-событие)
  и исчезает.

**Поведение**
- R7. Цель врага — ближайшая живая пешка игрока.
- R8. Враг держится в полосе дистанций [`MinDistance`, `MaxDistance`] от цели: ближе — отходит, дальше —
  подходит, внутри — стоит (с небольшим случайным смещением точки, чтобы не стоять в одной линии). Движение — по
  навмешу.
- R9. Враг всегда поворачивается к цели и стреляет, только если есть линия видимости от дула до цели
  (трасса `Visibility` не упирается в стену или другого врага); без видимости — прекращает огонь и подходит ближе.
- R10. Поведение собрано в StateTree (`ST_Enemy`) из C++-задач; логика задач — в тестируемых функциях /
  компонентах, задачи — тонкие обёртки.

**Координация (задел)**
- R11. Все живые враги регистрируются в подсистеме мира `UEnemyDirectorSubsystem` (появление / смерть); через
  неё враг находит цель. Сама координация (очереди на стрельбу, распределение позиций) — позже.

**Волны**
- R12. `AEnemyWaveSpawner` на уровне: список волн (класс врага × количество), точки появления (вокруг спавнера в
  радиусе или заданные точки), задержка перед волной. Следующая волна — через задержку после гибели всех врагов
  текущей; после последней волны список повторяется. Спавн — только на сервере.

**Шаблон**
- R13. Шаблонные C++-классы NPC (`Variant_Shooter/AI`: `ShooterNPC`, `ShooterAIController`, `ShooterNPCSpawner`,
  `ShooterStateTreeUtility`, `EnvQueryContext_Target`) удалены вместе с ассетами, которые без них не работают;
  контент, ставший неиспользуемым, перечислен как кандидат на удаление в
  [ContentCleanup.md](../ContentCleanup.md).

**Контент**
- R14. На `Lvl_FirstPerson` есть навмеш и спавнер волн; враг — `BP_Enemy` (манекен UE из шаблона), контроллер —
  `BP_EnemyAIController` со `ST_Enemy`.

**Вне рамок:** смерть и респавн игрока; анимации стрельбы, звуки, VFX; лут с врагов; разные типы врагов и
укрытия; координация группы (только задел R11); сетевая проверка; урон врагов друг другу специально не
запрещается (R9 не даёт стрелять через своих).

**Открытые вопросы:** нет.

## План итераций
- [x] 1. Удалить шаблонные NPC-классы и зависимые ассеты, составить список кандидатов на удаление (R13) —
  проверка: сборка, все тесты, `CompileAllBlueprints`
- [x] 2. Здоровье игрока и бессмертие (R1, R2) — тест: `TypicalShooter.Enemies.Health.*`, `.PlayerHealth.CharacterIsImmortal`
- [x] 3. Текст здоровья и виджет `UHealthCounterWidget` (R3) — тест: `TypicalShooter.Enemies.UI.HealthText`
  (сам виджет в `BP_HUD` ставится в итерации 11 — нужен открытый редактор)
- [x] 4. `AEnemyCharacter`: здоровье, урон, атака с базовыми параметрами (R4, R5) — тест: `TypicalShooter.Enemies.Character.*`
- [x] 5. Смерть врага (R6) — тест: `TypicalShooter.Enemies.Character.Dies*`
- [x] 6. Огонь только при линии видимости (R9) — тест: `TypicalShooter.Enemies.Combat.*`
- [x] 7. Выбор точки в полосе дистанций (R8) — тест: `TypicalShooter.Enemies.Positioning.*`
- [x] 8. `UEnemyDirectorSubsystem`: регистрация, поиск цели (R7, R11) — тест: `TypicalShooter.Enemies.Director.*`
- [x] 9. `AEnemyAIController` + задачи StateTree (цель, держать дистанцию, стрелять) (R8–R10) — тест:
  `TypicalShooter.Enemies.Controller.*` (логика задач); сами задачи в дереве — PIE-тест итерации 11
- [x] 10. `AEnemyWaveSpawner` (R12) — тест: `TypicalShooter.Enemies.Spawner.*`
- [ ] 11. Контент: `HealthCounter` в `BP_HUD`, `BP_Enemy`, `BP_EnemyAIController`, `ST_Enemy`, навмеш и спавнер на `Lvl_FirstPerson` (R14) —
  тест: `TypicalShooter.Enemies.Flow` (PIE: волна появляется, враги держат дистанцию и ранят игрока, игрок их
  убивает, приходит следующая волна)

## Как устроено
- Здоровье — [`UHealthComponent`](../../Source/TypicalShooter/Public/Shooting/HealthComponent.h) (общий для игрока и врагов):
  на сервере в `BeginPlay` выставляет `Health` / `MaxHealth` = `MaxHealth` в `UHealthAttributeSet` ASC владельца
  (набор и ASC создаёт сам актор); при падении здоровья до нуля один раз считает «поражение» и шлёт
  `OnDepleted` / `GetDepletedDelegate()`. `bRefillWhenDepleted` — на следующем тике восстановить здоровье (игрок пока
  бессмертен, R2), иначе остаётся на нуле (враг умирает по событию).
- Игрок (`ATypicalShooterCharacter`): `UHealthAttributeSet` + `UHealthComponent` (`bRefillWhenDepleted`) +
  `UGasDamageReceiverComponent`.
- HUD — [`UHealthCounterWidget`](../../Source/TypicalShooter/Public/UI/Widget/HealthCounterWidget.h) (как счётчик
  патронов): каждый кадр текст [`HealthDisplay::GetHealthText`](../../Source/TypicalShooter/Public/Shooting/HealthDisplay.h)
  по ASC пешки владельца — «Health: 73 / 100».
- Враг — [`AEnemyCharacter`](../../Source/TypicalShooter/Public/Enemies/EnemyCharacter.h): `UTS_GAS` (Minimal) +
  `UHealthAttributeSet` + `UAttackAttributeSet`, `UHealthComponent` (60, без восстановления), `UGasDamageReceiverComponent`,
  дуло `Muzzle` у капсулы (50 вперёд, 50 вверх). На сервере в `BeginPlay` выставляет базу основного слота из `AttackStats`
  (по умолчанию: очередь 3 × 0.15 с, пауза 1.5 с, 2500 см/с, 4 физ., конец огня 5000) и выдаёт `AttackAbility`
  (`UPrimaryAttackAbility`) с InputID `PrimaryAttack`. `SetFiring(true/false)` — «зажать / отпустить» атаку
  (`AbilityLocalInputPressed/Released`): пока зажато, абилка повторяет очереди.
- Смерть врага (`Die`, сервер, по `UHealthComponent::GetDepletedDelegate`): отпускает атаку и отменяет абилки,
  останавливает движение, выключает коллизию капсулы, отцепляется от контроллера, шлёт `OnDied` /
  `GetDiedDelegate()` и исчезает через `DeathLifeSpan` (0.1 с). Мёртвый враг не стреляет (`SetFiring` игнорируется).
- Решение стрелять — `CanShootAt(Target)`: цель в пределах конца огня основной атаки и трасса `Visibility` от дула до
  центра цели ничего не задевает (или задевает саму цель); `UpdateFiring(Target)` = `SetFiring(CanShootAt(Target))`.
  Капсула врага блокирует `Visibility` (профиль Pawn её игнорирует): прицел игрока останавливается на враге, а враги
  перекрывают линию огня друг другу.
- Куда встать — [`EnemyPositioning::ChooseStandPoint`](../../Source/TypicalShooter/Public/Enemies/EnemyPositioning.h)
  (`FEnemyStandQuery`): по горизонтали внутри полосы [Min, Max] и цель видна — стоять; иначе точка на середине полосы
  (без видимости — на её ближнем крае) со стороны врага, повёрнутая вокруг цели на случайный угол в пределах
  `JitterDegrees` (20°; без видимости — `FlankDegrees` 60°, чтобы зайти с другого угла), на высоте самого врага.
- Директор — [`UEnemyDirectorSubsystem`](../../Source/TypicalShooter/Public/Enemies/EnemyDirectorSubsystem.h)
  (`UWorldSubsystem`): враг регистрируется на сервере в `BeginPlay` и снимается при смерти и в `EndPlay`;
  `GetEnemies()` — живые враги, `FindTargetFor(Enemy)` — ближайшая пешка игрока (по контроллерам игроков). Сюда же
  ляжет координация группы.
- Контроллер — [`AEnemyAIController`](../../Source/TypicalShooter/Public/Enemies/EnemyAIController.h) (у врага
  `AIControllerClass`, `AutoPossessAI = PlacedInWorldOrSpawned`): `UStateTreeAIComponent` без автозапуска; при захвате
  пешки ставит в него `Behavior` (ассет StateTree) и запускает, при отпускании — останавливает. Шаги поведения:
  `UpdateTarget` (цель от директора + `SetFocus` — враг поворачивается и целится в центр цели), `UpdateFire`
  (`UpdateFiring(Target)`), `UpdatePosition(FEnemyStandSettings)` (когда стоит или прошло `RepathInterval` — выбирает
  точку `ChooseStandPoint` с видимостью от `CanShootAt` и идёт туда `MoveToLocation` по навмешу; без навданных — по
  прямой; дошёл до полосы по дороге — останавливается), `StopCombat`.
- Задачи StateTree — [EnemyStateTreeTasks.h](../../Source/TypicalShooter/Public/Enemies/EnemyStateTreeTasks.h) (категория
  «Enemy», контекст — `AIController`): `Enemy: Track Target`, `Enemy: Shoot When Visible` (на выходе отпускает атаку),
  `Enemy: Keep Distance` (параметры `FEnemyStandSettings`: полоса 800–1600, разброс 20° / 60°, на выходе останавливается).
  Все бегут бесконечно — кладутся в одно состояние и работают параллельно.
- Волны — [`AEnemyWaveSpawner`](../../Source/TypicalShooter/Public/Enemies/EnemyWaveSpawner.h) (сервер): `Waves` (список
  `FEnemyWaveEntry` = класс × количество; по умолчанию 2, 3, 4 `AEnemyCharacter`), первая волна через `FirstWaveDelay`,
  следующая — через `WaveDelay` после гибели (или уничтожения) последнего врага текущей, после последней — снова первая.
  Точки появления — `SpawnPoints` по очереди или случайная точка в круге `SpawnRadius` вокруг спавнера (над точкой на
  полвысоты капсулы, случайный поворот). `OnWaveStarted(WaveIndex)`, `GetWaveIndex` / `GetWavesStarted` / `GetAliveCount`.

## Настройка контента
Будет заполнено по ходу итераций.

## Тесты
[EnemiesHealthTest.cpp](../../Source/TypicalShooter/Private/Tests/EnemiesHealthTest.cpp) (мир без PIE):
`TypicalShooter.Enemies.Health.StartsFull`, `.RefillsWhenDepleted`, `.StaysDepleted`,
`TypicalShooter.Enemies.PlayerHealth.CharacterIsImmortal` (компоненты на CDO персонажа).
[EnemiesHealthDisplayTest.cpp](../../Source/TypicalShooter/Private/Tests/EnemiesHealthDisplayTest.cpp): `TypicalShooter.Enemies.UI.HealthText`.
[EnemiesCharacterTest.cpp](../../Source/TypicalShooter/Private/Tests/EnemiesCharacterTest.cpp) (мир с полом, фикстура
[EnemiesTestFixture.h](../../Source/TypicalShooter/Private/Tests/EnemiesTestFixture.h)): `TypicalShooter.Enemies.Character.Defaults`,
`.TakesDamage`, `.FiresWhileFiring` (очереди по манекену, после отпускания новых снарядов нет).
[EnemiesDeathTest.cpp](../../Source/TypicalShooter/Private/Tests/EnemiesDeathTest.cpp): `TypicalShooter.Enemies.Character.DiesAtZeroHealth`,
`.DiesAndStopsFiring`.
[EnemiesCombatTest.cpp](../../Source/TypicalShooter/Private/Tests/EnemiesCombatTest.cpp): `TypicalShooter.Enemies.Combat.FiresWithLineOfSight`,
`.WallBlocksFire`, `.AllyBlocksFire`, `.OutOfRange`.
[EnemiesPositioningTest.cpp](../../Source/TypicalShooter/Private/Tests/EnemiesPositioningTest.cpp): `TypicalShooter.Enemies.Positioning.InsideBandStays`,
`.TooCloseBacksOff`, `.TooFarApproaches`, `.NoLineOfSightApproaches`, `.OnTopOfTarget`.
[EnemiesDirectorTest.cpp](../../Source/TypicalShooter/Private/Tests/EnemiesDirectorTest.cpp): `TypicalShooter.Enemies.Director.RegistersLiveEnemies`,
`.NearestPlayerTarget`.
[EnemiesControllerTest.cpp](../../Source/TypicalShooter/Private/Tests/EnemiesControllerTest.cpp) (мир с навигационной системой
 без навмеша — `EnemiesTest::AddNavigation`): `TypicalShooter.Enemies.Controller.PossessesSpawnedEnemy`, `.TracksAndShootsTarget`,
`.KeepsDistance` (слишком близко → отходит в полосу и там стоит).
[EnemiesSpawnerTest.cpp](../../Source/TypicalShooter/Private/Tests/EnemiesSpawnerTest.cpp): `TypicalShooter.Enemies.Spawner.FirstWaveAfterDelay`,
`.NextWaveWhenAllDead` (и повтор списка), `.UsesSpawnPoints`.

## Ограничения и грабли
- В «голом» тестовом мире нет навигационной системы: `MoveToLocation` не стартует даже по прямой. Тесты движения
  вызывают `EnemiesTest::AddNavigation` (система без навданных — прямые перемещения работают).

## Открытые задачи
- [ ] Координация группы через `UEnemyDirectorSubsystem`.
- [ ] Смерть игрока (сейчас бессмертен).

---
_Сверено с кодом: 2026-11-26_
