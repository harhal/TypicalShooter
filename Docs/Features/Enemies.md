# Враги: стрелки, волны, ИИ на StateTree

**Статус:** ✅ Работает (влито в `main`): на `Lvl_FirstPerson` волны врагов-стрелков держат дистанцию, стреляют
по игроку при линии видимости и умирают; игрок пока бессмертен. Мультиплеер не проверялся.

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
- [x] 11a. `ST_Enemy` генерируется из кода коммандлетом `BuildEnemyStateTree` (в MCP StateTree-ассеты только читаются)
  — тест: `TypicalShooter.Enemies.Behavior.TreeCompiles`
- [x] 11. Контент: `HealthCounter` в `BP_HUD`, `BP_Enemy`, `BP_EnemyAIController`, `ST_Enemy`, навмеш и спавнер на `Lvl_FirstPerson` (R14) —
  тест: `TypicalShooter.Enemies.Flow` (PIE: волна появляется, дерево ведёт игрока, враги держат дистанцию и ранят
  игрока, враг у стены отходит от подошедшего игрока по навмешу, после гибели волны приходит следующая)

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
  (`AbilityLocalInputPressed/Released`): пока зажато, абилка повторяет очереди. У `CharacterMovement` включено RVO-избегание
  (`bUseRVOAvoidance`, радиус учёта 400, вес 0.5): следование по пути пешек не видит, и стоящий на пути союзник иначе
  навсегда останавливал врага.
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
  `ChooseReachableStandPoint` — то же, но точка должна пройти проверку (контроллер проецирует её на навмеш); иначе она
  поворачивается вокруг цели дальше (±30°…180°) — враг у стены отходит вбок, а не в стену. Если координатор выдал
  сектор (`SectorYaw`), точка строится от него и не выходит за его пределы — см. [EnemyCoordination.md](EnemyCoordination.md).
- Директор — [`UEnemyDirectorSubsystem`](../../Source/TypicalShooter/Public/Enemies/EnemyDirectorSubsystem.h)
  (`UWorldSubsystem`): враг регистрируется на сервере в `BeginPlay` и снимается при смерти и в `EndPlay`;
  `GetEnemies()` — живые враги, `FindTargetFor(Enemy)` — ближайшая пешка игрока (по контроллерам игроков). Он же
  координирует группы врагов по цели (роли, сектора, токены атаки) — см. [EnemyCoordination.md](EnemyCoordination.md).
- Контроллер — [`AEnemyAIController`](../../Source/TypicalShooter/Public/Enemies/EnemyAIController.h) (у врага
  `AIControllerClass`, `AutoPossessAI = PlacedInWorldOrSpawned`): `UStateTreeAIComponent` без автозапуска; при захвате
  пешки ставит в него `Behavior` (ассет StateTree) и запускает, при отпускании — останавливает. Шаги поведения:
  `UpdateTarget` (цель от директора + `SetFocus` — враг поворачивается и целится в центр цели), `UpdateFire`
  (стреляет, только если держит токен атаки группы и `CanAttack(Target)`), `UpdatePosition(FEnemyStandSettings)` (когда
  стоит или прошло `RepathInterval` — выбирает точку `ChooseReachableStandPoint` (проекция на навмеш, без навданных —
  любая) с видимостью от `CanShootAt` и идёт туда `MoveToLocation` по навмешу; без навданных — по прямой; дошёл до полосы
  по дороге — останавливается). Если директор координирует врага, полоса берётся из роли, а точка — из сектора
  (`EnemyRoleStand::Apply`); `Min/MaxDistance` из `FEnemyStandSettings` работают только без координации. `StopCombat`
  отдаёт токен. Подробности — [EnemyCoordination.md](EnemyCoordination.md).
- Задачи StateTree — [EnemyStateTreeTasks.h](../../Source/TypicalShooter/Public/Enemies/EnemyStateTreeTasks.h) (категория
  «Enemy», контекст — `AIController`): `Enemy: Track Target`, `Enemy: Shoot When Visible` (на выходе отпускает атаку),
  `Enemy: Keep Distance` (параметры `FEnemyStandSettings`: полоса 800–1600, разброс 20° / 60°, на выходе останавливается).
  Все бегут бесконечно — кладутся в одно состояние и работают параллельно.
- Дерево `/Game/Enemies/AI/ST_Enemy` (схема StateTree AI Component) собирается из кода:
  [`UBuildEnemyStateTreeCommandlet`](../../Source/TypicalShooter/Public/Enemies/BuildEnemyStateTreeCommandlet.h) —
  Root → Combat {Track Target, Shoot When Visible} → по состоянию на роль (`Pressure` / `Flank` / `Cover`, условие
  `Enemy: Has Role`) и `Free`, в каждом Keep Distance (схема — в [EnemyCoordination.md](EnemyCoordination.md)); контекст
  задач и условий привязывается автоматически по типу.
- Волны — [`AEnemyWaveSpawner`](../../Source/TypicalShooter/Public/Enemies/EnemyWaveSpawner.h) (сервер): `Waves` (список
  `FEnemyWaveEntry` = класс × количество; по умолчанию 2, 3, 4 `AEnemyCharacter`), первая волна через `FirstWaveDelay`,
  следующая — через `WaveDelay` после гибели (или уничтожения) последнего врага текущей, после последней — снова первая.
  Точки появления — `SpawnPoints` по очереди или случайная точка в круге `SpawnRadius` вокруг спавнера (над точкой на
  полвысоты капсулы, случайный поворот). `OnWaveStarted(WaveIndex)`, `GetWaveIndex` / `GetWavesStarted` / `GetAliveCount`.
  Консольная переменная `TypicalShooter.Enemies.Waves 0` — спавнеры, начавшие игру, волн не запускают (отладка; PIE-тесты
  не про врагов выключают волны через [EnemyWavesTestGuard.h](../../Source/TypicalShooter/Private/Tests/EnemyWavesTestGuard.h)).

## Настройка контента
Дерево поведения меняем в коде (`BuildTree`) и перегенерируем (редактор закрыт):
`UnrealEditor-Cmd.exe <uproject> -run=BuildEnemyStateTree -unattended -nopause -NullRHI`.
Правки дерева руками в редакторе затрутся при следующей генерации. Параметры полосы дистанций — в `BuildTree`
(`FEnemyStandSettings` задачи Keep Distance).

Ассеты — `Content/Enemies`:
- `Blueprints/BP_Enemy` (от `AEnemyCharacter`): меш `SKM_Manny_Simple` + `ABP_Unarmed` (Z −90, Yaw −90),
  `AIControllerClass = BP_EnemyAIController`. Здоровье — компонент `Health`, атака — `Attack Stats` / `Attack Ability`.
- `Blueprints/BP_EnemyAIController` (от `AEnemyAIController`): `Behavior = ST_Enemy`.
- `AI/ST_Enemy` — сгенерирован (см. выше).

Уровень `Lvl_FirstPerson`:
- `NavMeshBounds_Arena` — на всю арену (центр (0, 0, 300), масштаб 22 × 22 × 8). Навмеш строится в рантайме:
  `DefaultEngine.ini` → `[/Script/NavigationSystem.RecastNavMesh] RuntimeGeneration=Dynamic` (уровень с OFPA, запечённых
  навданных нет).
- `EnemyWaveSpawner_Back` в (−1500, 0, 0) — за спиной игрока у дальнего края: волны 2 / 3 / 4 `BP_Enemy`, первая через
  2 с, следующие через 3 с, радиус появления 300.

HUD: `BP_HUD` → `HealthCounter` (`UHealthCounterWidget`), левый нижний угол (отступ 40), авторазмер.

Отладка: `TypicalShooter.Enemies.Waves 0` в консоли до старта PIE — арена без врагов.

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
`.TooCloseBacksOff`, `.TooFarApproaches`, `.NoLineOfSightApproaches`, `.TurnsAwayFromUnreachablePoints`, `.OnTopOfTarget`.
[EnemiesDirectorTest.cpp](../../Source/TypicalShooter/Private/Tests/EnemiesDirectorTest.cpp): `TypicalShooter.Enemies.Director.RegistersLiveEnemies`,
`.NearestPlayerTarget`.
[EnemiesControllerTest.cpp](../../Source/TypicalShooter/Private/Tests/EnemiesControllerTest.cpp) (мир с навигационной системой
 без навмеша — `EnemiesTest::AddNavigation`): `TypicalShooter.Enemies.Controller.PossessesSpawnedEnemy`, `.TracksAndShootsTarget`,
`.KeepsDistance` (слишком близко → отходит в полосу и там стоит).
[EnemiesSpawnerTest.cpp](../../Source/TypicalShooter/Private/Tests/EnemiesSpawnerTest.cpp): `TypicalShooter.Enemies.Spawner.FirstWaveAfterDelay`,
`.NextWaveWhenAllDead` (и повтор списка), `.UsesSpawnPoints`, `.DisabledByConsoleVariable`.
[EnemiesAvoidanceTest.cpp](../../Source/TypicalShooter/Private/Tests/EnemiesAvoidanceTest.cpp): `TypicalShooter.Enemies.Movement.AvoidsStandingAlly`
(враг обходит союзника, стоящего на прямой к цели; без RVO застревал в 6.7 м от неё).
[EnemiesBehaviorTreeTest.cpp](../../Source/TypicalShooter/Private/Tests/EnemiesBehaviorTreeTest.cpp): `TypicalShooter.Enemies.Behavior.TreeCompiles`.
[EnemiesFlowTest.cpp](../../Source/TypicalShooter/Private/Tests/EnemiesFlowTest.cpp) — PIE `TypicalShooter.Enemies.Flow` на
`Lvl_FirstPerson`: первая волна, дерево ведёт игрока, враг в полосе дистанций своей роли (±150), игрок ранен, игрок
телепортируется в 3 м от врага у стены — враг отходит по навмешу (до ближнего края полосы своей роли − 150), волна убита —
приходит следующая. Пишет в лог состояние навмеша и
движения отходящего врага. Остальные PIE-тесты выключают волны (`EnemyWavesTestGuard`).

Видео: [RecordingEnemyKillTest.cpp](../../Source/TypicalShooter/Private/Tests/RecordingEnemyKillTest.cpp) —
`TypicalShooterRecordings.EnemyKill` (вне наборов `TypicalShooter.`): PIE с фиксированным шагом 1/30 с, игрок
разворачивается к ближайшему врагу первой волны и расстреливает его, каждый кадр — в `Saved/Recordings/EnemyKill/`.
Запуск — только в редакторе с рендерингом (не `-NullRHI`), затем ffmpeg (кадр — всё окно редактора, вьюпорт вырезается
`crop`; координаты зависят от раскладки окна):
`UnrealEditor.exe <uproject> "-ExecCmds=Automation RunTests TypicalShooterRecordings.EnemyKill;Quit" -unattended -nopause`,
`ffmpeg -framerate 30 -i Saved/Recordings/EnemyKill/frame_%04d.png -vf crop=1160:604:4:134 -c:v libx264 -pix_fmt yuv420p EnemyKill.mp4`.

## Ограничения и грабли
- В «голом» тестовом мире нет навигационной системы: `MoveToLocation` не стартует даже по прямой. Тесты движения
  вызывают `EnemiesTest::AddNavigation` (система без навданных — прямые перемещения работают).
- StateTree-тулсет Unreal MCP только читает деревья — поэтому `ST_Enemy` генерируется коммандлетом.
- При загрузке уровня `LogNavigation` предупреждает «Recreating dtNavMesh instance» — сохранённый `RecastNavMesh` пуст,
  навмеш пересобирается в рантайме (динамическая генерация); безвредно.
- Враги не различают «своих»: снаряд врага ранит другого врага, если тот влезет на линию огня после выстрела
  (`CanShootAt` проверяет линию только в момент решения стрелять).
- RVO учитывает только агентов с избеганием (враги). Игрок к ним не относится, так что врага на пути через игрока
  останавливает обычная коллизия капсул.

## Открытые задачи
- [ ] Координация группы через `UEnemyDirectorSubsystem` — в работе: [EnemyCoordination.md](EnemyCoordination.md).
- [ ] Смерть игрока (сейчас бессмертен).
- [ ] Анимации стрельбы / смерти, звук, VFX; оружие в руках у манекена.
- [ ] Разные типы врагов (свои `AttackStats`, паттерны, полосы дистанций).
- [ ] Проверить врагов по сети (репликация движения и снарядов, клиент).

---
_Сверено с кодом: 2026-11-26_
