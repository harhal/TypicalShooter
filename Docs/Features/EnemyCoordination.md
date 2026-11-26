# Координатор противников: очередь огня, роли, окружение

**Статус:** 🟡 В работе (ветка `feature/enemy-coordinator`): требования и план согласованы.

## Что это
Враги перестают действовать поодиночке и воюют группой. Право стрелять раздаётся по очереди: одновременно
по игроку стреляют несколько врагов, а не все, кто его видит. У каждого врага есть роль: **давящий** подходит
близко, **фланкёр** заходит сбоку или со спины (вне поля зрения игрока), **прикрывающий** держится далеко.
Группа расходится по кругу вокруг игрока, а не толпится в одной точке. Решения принимает «директор»
(`UEnemyDirectorSubsystem`) на сервере. Если игроков несколько, у каждого своя группа: координируются те
враги, которые выбрали его целью.

Развивает задел R11 из [Enemies.md](Enemies.md) (там описаны сам враг, его StateTree и волны).

## Требования

**Группы**
- R1. Директор (сервер) объединяет живых врагов в группы по цели: враги, которым `FindTargetFor` дал одну и ту же
  пешку игрока, — одна группа. Враг без цели ни в какую группу не входит. Каждая группа координируется отдельно:
  роли, сектора и токены одной группы не зависят от других.
- R2. Группы пересчитываются с периодом `UpdateInterval` (из настроек), а также сразу, когда враг регистрируется,
  погибает или меняет цель.

**Очередь на стрельбу (токены атаки)**
- R3. Число токенов группы считается так: `N = clamp(round(TokenFraction × размер группы), MinTokens, MaxTokens)`.
  Значения по умолчанию: 40 %, 1–3.
- R4. Токен — это право на любую атаку, а не только на стрельбу: очередь стрелка, удар бойца ближнего боя, рывок
  камикадзе к цели. Враг атакует, только если держит токен и его атака возможна (у стрелка — `CanShootAt(Target)`).
  Без токена он не атакует, даже когда видит цель.
- R5. Токен выдаётся только врагу, который видит цель. Если желающих больше, чем свободных токенов, порядок такой:
  сначала по приоритету роли (фиксированный порядок: давящий > фланкёр > прикрывающий), затем тот, кто дольше ждёт,
  затем ближайший к цели.
- R6. Ротация: враг держит токен не дольше `TokenHoldTime` (по умолчанию 3 с). Раньше он отдаёт токен, если
  потерял видимость дольше `LostSightGrace` (0.5 с), умер или вышел из боя (выход из состояния StateTree). Отдав
  токен, враг ждёт `TokenCooldown` (1.5 с), прежде чем снова может его получить. Исключение: в группе нет других
  претендентов.

**Роли**
- R7. Роли: `Pressure` (давящий), `Flank` (фланкёр), `Cover` (прикрывающий). Каждой роли в настройках задана полоса
  дистанций [Min, Max]. По умолчанию: давящий 500–900, фланкёр 700–1200, прикрывающий 1400–2000.
- R8. У класса врага есть список разрешённых ролей `AllowedRoles` (по умолчанию все три). Директор выдаёт врагу
  только разрешённую роль.
- R9. Директор старается выдержать в группе желаемые доли ролей (`RoleWeights` в настройках; по умолчанию 2 : 1 : 1).
  Доля роли не может оказаться перебрана, если другой враг мог бы её закрыть. Распределение устойчиво: при
  пересчёте враг сохраняет роль, если доли от этого не нарушаются. Когда враг появляется или погибает, роли
  меняют как можно меньше врагов.
- R10. Поведение без токена зависит от роли. Давящий сокращает дистанцию до ближнего края своей полосы. Фланкёр
  идёт в свой фланговый сектор. Прикрывающий стоит на месте, пока видит цель. С токеном все трое держат свою
  полосу и стреляют (R4).

**Позиции (сектора, окружение)**
- R11. Каждому врагу группы директор выдаёт угол вокруг цели (сектор). Враг встаёт на этом угле в полосе своей роли
  с разбросом в пределах ширины сектора. Проверка достижимости по навмешу остаётся прежней: если точка не
  годится, враг пробует повернуть её дальше.
- R12. Сектора давящих и прикрывающих раздаются равномерно по кругу. Сектора фланкёров лежат вне конуса взгляда
  игрока (`FlankViewHalfAngle`, по умолчанию 60° от направления камеры по горизонтали), то есть сбоку или сзади.
- R13. Распределение секторов устойчиво: каждый враг получает ближайший к своему текущему углу свободный сектор,
  чтобы враги не бегали вокруг игрока. Фланговые сектора пересчитываются, только когда взгляд игрока повернулся
  больше чем на `FlankReaimDegrees` (30°) от последнего пересчёта (гистерезис).
- R14. Две выданные точки разных врагов не оказываются ближе `MinSeparation` (по умолчанию 250 см) друг к другу, если
  в полосе есть место. Это следствие секторов, которое проверяется тестом.

**StateTree**
- R15. Combat в `ST_Enemy` получает дочерние состояния `Pressure`, `Flank` и `Cover`. Активное состояние выбирается
  условием `Enemy: Has Role`. В Combat работают Track Target и стрельба (теперь с токеном), в дочерних
  состояниях — Keep Distance с параметрами своей роли. Если роль меняется, дерево переходит в другое состояние.
  Дерево по-прежнему генерируется коммандлетом `BuildEnemyStateTree`.

**Настройка и отладка**
- R16. Все параметры (R2, R3, R5–R7, R9, R12–R14) собраны в `UEnemyCoordinationSettings` (Project Settings → Game →
  Enemy Coordination, `DefaultGame.ini`), по образцу `ULootSettings`.
- R17. Консольная переменная `TypicalShooter.Enemies.Debug.Coordination 1` включает отрисовку на сервере. Над
  каждым врагом выводятся роль и отметка токена, от врага проводится линия к выданной точке, вокруг цели
  рисуются лучи секторов и конус взгляда для фланкёров.

**Типы врагов**
- R18. Будущие типы врагов, включая ближний бой и камикадзе, покрываются тремя ролями без новых механизмов
  координации. Для этого у класса врага, кроме `AllowedRoles`, есть переопределения полос дистанций по ролям
  (`RoleBandOverrides`; если переопределения нет, берётся полоса из настроек). Например, боец ближнего боя — это
  `Pressure` / `Flank` с полосой 0–150 см. Камикадзе — `Pressure` / `Flank`, который без токена держится на краю полосы,
  а получив токен, бежит к цели. Координатор не знает, какая у врага атака: он раздаёт роли, сектора и токены, а
  «что значит атаковать» решает сам враг. В этой фиче проверяется только механизм переопределения; самих бойцов
  ближнего боя и камикадзе в ней нет.

**Вне рамок:** укрытия и тактическая точка обзора (EQS); реализация новых типов врагов: ближнего боя, камикадзе и
других (готовим только задел R18); бои с боссами (позже, отдельной фичей); общение врагов, крики и анимации реакции; распределение врагов между игроками (каждый враг
по-прежнему берёт ближайшего); проверка по сети: координация работает только на сервере, реплицировать нечего, но
клиент PIE не проверяется; запрет огня по своим (как и раньше, остаётся линия видимости из `CanShootAt`).

**Открытые вопросы:**
- Должен ли токен отбираться у врага, который держит его, но перестал видеть цель (сейчас: через `LostSightGrace`)?
  Или токен «бронируется» на время подхода?
- Нужно ли масштабировать `MaxTokens` от сложности или номера волны? Пока это константа в настройках.

## План итераций
- [x] 1. `UEnemyCoordinationSettings` + число токенов от размера группы (R3, R16) — тест:
  `TypicalShooter.Enemies.Coordination.TokenCount`
- [x] 2. Распределение ролей, чистая функция (разрешённые роли, доли, устойчивость) (R7–R9; полосы дистанций ролей из R7
  добавятся в итерации 7, вместе с их использованием) — тест: `TypicalShooter.Enemies.Coordination.Roles.*` (`.Quotas`,
  `.RespectsWeights`, `.OnlyAllowed`, `.NoOvershootWhenAnotherCanCover`, `.StableOnRecompute`, `.MinimalChangeOnDeath`)
- [x] 3. Раздача секторов, чистая функция (равномерно, ближайший свободный, фланкёры вне конуса взгляда, гистерезис)
  (R12, R13) — тест: `TypicalShooter.Enemies.Coordination.Sectors.*` (`.EvenSpread`, `.NearestFree`, `.FlankersOutOfView`,
  `.ReaimHysteresis`)
- [x] 4. Раздача токенов, чистая логика со временем (приоритет, удержание, кулдаун, потеря видимости) (R5, R6) — тест:
  `TypicalShooter.Enemies.Coordination.Tokens.*` (`.OnlyVisible`, `.Priority`, `.RotationAfterHoldTime`, `.LostSightGrace`,
  `.CooldownAfterRelease`)
- [x] 5. Точка в заданном секторе и полосе роли (`FEnemyStandQuery` + угол сектора), разнесение точек (R11, R14) — тест:
  `TypicalShooter.Enemies.Positioning.SectorAngleMovesIntoSector`, `.SectorAngleStaysInside`, `.Separation`
- [x] 6. Директор: группы по цели, периодический пересчёт (tickable subsystem), API роли / сектора / токена (R1, R2) —
  тест: `TypicalShooter.Enemies.Director.GroupsTwoTargets`, `.GroupsSpreadAndShareTokens`, `.RecomputeOnDeath`,
  `.FollowsTargetChange`
- [x] 7. Враг и контроллер: `AllowedRoles`, `RoleBandOverrides`, огонь только с токеном, позиция по роли и сектору, отдача
  токена при выходе из боя и смерти (R4, R8, R10, R18) — тест: `TypicalShooter.Enemies.Controller.FiresOnlyWithToken`,
  `.RoleBand`, `.RoleBandOverride`; поведение роли — `TypicalShooter.Enemies.Coordination.RoleStand.*`
- [x] 8. StateTree: условие `Enemy: Has Role`, состояния ролей, перегенерация `ST_Enemy` (R15) — тест:
  `TypicalShooter.Enemies.Behavior.TreeCompiles`, `.RoleStates`
- [ ] 9. Отладочная отрисовка (R17) — ручная проверка в PIE (описать в доке), плюс тест, что при выключенной переменной
  отрисовки нет: `TypicalShooter.Enemies.Coordination.DebugDrawOff`
- [ ] 10. PIE-сценарий на `Lvl_FirstPerson`: волна из 4 врагов, одновременно стреляют не больше N, роли распределены по
  долям, враги разошлись по углу (минимальный угол между ними), фланкёр вне конуса взгляда игрока, после гибели врага роли
  перераспределяются — тест: `TypicalShooter.Enemies.Coordination.Flow`

## Как устроено
Код координации лежит в `Source/TypicalShooter/{Public,Private}/Enemies/Coordination/`.

Готово:
- Настройки — [`UEnemyCoordinationSettings`](../../Source/TypicalShooter/Public/Enemies/Coordination/EnemyCoordinationSettings.h)
  (`UDeveloperSettings`, `Config = Game`, хранится в `DefaultGame.ini`). Группа `Tokens` —
  [`FEnemyTokenCountSettings`](../../Source/TypicalShooter/Public/Enemies/Coordination/EnemyTokenCount.h) (доля, минимум,
  максимум).
- Число токенов — `EnemyCoordination::TokenCount(GroupSize, Settings)`: `round(доля × размер)` в пределах [Min, Max], но
  не больше размера группы; у пустой группы 0. Обёртка на настройках — `GetTokenCount(GroupSize)`.
- Роли — [`EEnemyRole`](../../Source/TypicalShooter/Public/Enemies/Coordination/EnemyRole.h) (`Pressure`, `Flank`,
  `Cover`) и веса `FEnemyRoleWeights` (настройка `RoleWeights`, 2 : 1 : 1).
- Распределение ролей — [`EnemyRoles`](../../Source/TypicalShooter/Public/Enemies/Coordination/EnemyRoleAssignment.h):
  - `ComputeQuotas(N, Weights)` делит N по весам методом наибольшего остатка. При равных остатках выигрывает роль,
    которая раньше в перечислении. Роль с весом 0 квоты не получает. Если все веса нулевые, все враги давящие.
  - `Assign(Candidates, Weights)` принимает кандидатов `{Allowed — маска ролей (0 = любая), Current — прошлая роль}`.
    Сначала каждый сохраняет текущую роль, если она разрешена и квота не перебрана. Остальных обрабатывает по
    порядку, начиная с самых ограниченных (меньше разрешённых ролей). Каждый получает разрешённую роль, где больше
    всего свободных мест. Если таких нет, поиск в ширину по ролям ищет цепочку сдвигов: гибкий враг переходит в роль
    со свободным местом и освобождает место для ограниченного. Только если сдвиг невозможен, роль перебирается
    (выбирается наименее переполненная).
- Сектора — [`EnemySectors`](../../Source/TypicalShooter/Public/Enemies/Coordination/EnemySectors.h). Все углы — это yaw
  в градусах вокруг цели в мировых осях (`YawAround(Target, Point)`).
  - `Assign({Members: роль + текущий угол, ViewYaw, FlankViewHalfAngle})` ставит N равноотстоящих слотов. Их фаза
    подгоняется под текущие углы врагов (круговое среднее N·θ, делённое на N), поэтому уже разошедшаяся группа
    остаётся на месте, а сбившаяся в кучу расходится с минимальными перебежками.
  - Первыми слоты получают фланкёры, только вне конуса взгляда (`|угол − ViewYaw| > FlankViewHalfAngle`). Если таких
    слотов меньше, чем фланкёров, слоты переставляются так, чтобы один встал строго за спиной цели. Остальные враги и
    фланкёры, которым не хватило места, занимают слоты жадно, начиная с ближайших пар «враг — слот».
  - Сектора общие для всех ролей: роли различаются полосой дистанций, а не кругом.
  - `SectorHalfWidth(N)` = 180 / N — в его пределах враг разбросает точку (итерация 5).
  - `ShouldReaimFlank(Опорный, Текущий, Порог)` — гистерезис. Директор хранит опорный взгляд цели и передаёт его в
    `Assign` как `ViewYaw`, а обновляет его, только когда взгляд ушёл дальше порога.
- Токены во времени — [`FEnemyTokenPool`](../../Source/TypicalShooter/Public/Enemies/Coordination/EnemyTokenPool.h), по
  одному на группу. `Update(Now, Candidates {Id, Role, bSeesTarget, Distance}, TokenCount, Settings)` проходит четыре шага:
  1. Синхронизация. Кто пропал из списка, уходит вместе со своим токеном. Новичок «ждёт» с момента появления.
     Отслеживается, с какого момента враг не видит цель.
  2. Возврат. Держатель отдаёт токен, если не видит цель дольше `LostSightGrace`, или если держит его дольше
     `TokenHoldTime` и есть другой претендент (видит цель и не держит токен).
  3. Лишние токены (группа уменьшилась) отдают держатели с самым низким приоритетом.
  4. Раздача свободных токенов претендентам. Порядок: роль (Pressure, Flank, Cover), время ожидания, дистанция, Id.
     Враги вне кулдауна (`TokenCooldown` после возврата) идут раньше. Враг на кулдауне получает токен, только если других
     претендентов нет.

  `Release(Id, Now)` — немедленный возврат (выход из боя, смерть). Отдавший токен «ждёт» с момента возврата. Поэтому при
  ротации токен уходит тому, кто ждал дольше, а не обратно ближайшему.
- Точка в секторе — [`EnemyPositioning`](../../Source/TypicalShooter/Public/Enemies/EnemyPositioning.h). У
  `FEnemyStandQuery` появились необязательные `SectorYaw`, `SectorHalfWidth` и `MinSeparation`. Если сектор задан:
  - враг стоит на месте, только если он в полосе, видит цель и находится внутри своего сектора;
  - иначе точка строится от угла сектора (а не от стороны врага), на прежнем радиусе: середина полосы или ближний край
    без видимости;
  - поворот точки (разброс или фланговый заход) ограничен полушириной сектора минус половина угла, который
    `MinSeparation` занимает на этом радиусе. Поэтому точки соседних секторов не ближе `MinSeparation`, пока сектора
    достаточно широкие; в слишком узких секторах разброса нет;
  - поиск достижимой точки (`ChooseReachableStandPoint`) поворачивает её дальше от угла сектора, как и раньше.

  Без сектора поведение прежнее.
- Одна группа — [`FEnemyGroupCoordinator`](../../Source/TypicalShooter/Public/Enemies/Coordination/EnemyGroupCoordinator.h)
  (без мира). `Update(Now, место цели, взгляд цели, участники {Id, AllowedRoles, Location, bCanAttack}, Settings)` по
  очереди считает роли (`EnemyRoles::Assign` с прошлыми ролями), сектора (`EnemySectors::Assign`; опорный взгляд для
  фланга обновляется по `ShouldReaimFlank`) и токены (`FEnemyTokenPool`, число — `GetTokenCount(размер)`). Результат —
  `Find(Id)` → {Role, SectorYaw, SectorHalfWidth}, плюс `HasToken` / `ReleaseToken`.
- Директор — [`UEnemyDirectorSubsystem`](../../Source/TypicalShooter/Public/Enemies/EnemyDirectorSubsystem.h), теперь
  `UTickableWorldSubsystem`.
  - `Recompute()` группирует живых врагов по `FindTargetFor` и обновляет каждую группу. Координатор цели, у которой уже
    была группа, переносится, поэтому токены, кулдауны и опорный взгляд не сбрасываются.
  - Пересчёт идёт сразу при `Register` / `Unregister` (появление, смерть, `EndPlay`), на тике, если у какого-то врага
    сменилась ближайшая цель, и раз в `UpdateInterval`.
  - `Id` врага — `GetUniqueID()`, `bCanAttack` — `CanShootAt(Target)`, взгляд цели — `GetViewRotation().Yaw`.
    `AllowedRoles` пока 0 (любая роль) — итерация 7.
  - API: `GetAssignment(Enemy)` → `FEnemyAssignment` (роль, сектор, `Target`; null без цели), `HasAttackToken`,
    `ReleaseAttackToken`, `GetGroupTargets`, `GetGroupMembers(Target)`. `GetEnemies()` больше не возвращает мёртвых
    (между `Die` и уничтожением актора).
- Поведение роли — [`EnemyRoleStand::Apply`](../../Source/TypicalShooter/Public/Enemies/Coordination/EnemyRoleStand.h)
  (`FEnemyDistanceBand`, `FEnemyRoleBands`). Переводит {роль, есть ли токен, полоса роли, сектор, `MinSeparation`} в
  `FEnemyStandQuery`:
  - давящий без токена держится в ближней половине полосы (`MaxDistance` = середина), то есть подходит;
  - прикрывающий без токена получает `bSectorRequiredToStay = false`: стоит в любом месте полосы, пока видит цель, а в
    сектор идёт, только если всё равно пришлось двигаться;
  - фланкёр и все с токеном держат свою полосу и свой сектор.
- Враг — [`AEnemyCharacter`](../../Source/TypicalShooter/Public/Enemies/EnemyCharacter.h) (роли в `EnemyCharacterRoles.cpp`):
  - `AllowedRoles` (по умолчанию все три) → `GetAllowedRoleMask()` для директора;
  - `RoleBandOverrides` (роль → полоса) → `GetRoleBand(Role)`: переопределение класса, иначе полоса из настроек;
  - виртуальный `CanAttack(Target)` (по умолчанию `CanShootAt`): по нему директор решает, кому можно дать токен. Боец
    ближнего боя или камикадзе переопределит его своей дальностью (R18).
- Контроллер — [`AEnemyAIController`](../../Source/TypicalShooter/Public/Enemies/EnemyAIController.h):
  - `UpdateFire` = `SetFiring(HasAttackToken() && CanAttack(Target))`;
  - `UpdatePosition` после базового запроса вызывает `ApplyCoordination`: роль, токен, полоса и сектор от директора.
    Без назначения (нет цели) остаются полоса из `FEnemyStandSettings` и сторона самого врага;
  - `ReleaseAttackToken()` вызывается из `StopCombat` и при выходе из задачи `Enemy: Shoot When Visible` (R6, выход из боя).
    Смерть отдаёт токен через `Unregister`. Пока контроллер или мир разрушаются (сборка мусора, конец PIE), директора уже
    нет, и `ReleaseAttackToken` ничего не делает.
  - `GetCoordinatedRole(OutRole)` — роль от директора (false без координации); `GetActiveStateNames()` — активные
    состояния StateTree (отладка, тесты).
- StateTree `ST_Enemy` (генерирует `UBuildEnemyStateTreeCommandlet::BuildTree` / `AddRoleStates`):
  ```
  Root
  └─ Combat            Track Target, Shoot When Visible (с токеном)
     ├─ Pressure       вход: Enemy: Has Role(Pressure); Keep Distance
     ├─ Flank          вход: Enemy: Has Role(Flank);    Keep Distance
     ├─ Cover          вход: Enemy: Has Role(Cover);    Keep Distance
     └─ Free           без условия (нет координации);   Keep Distance (полоса из параметров задачи)
  ```
  Из каждого состояния роли по тику есть переход к каждому другому состоянию роли с условием `Has Role(его роль)` и в
  `Free` с условием `Enemy: Not Coordinated`; из `Free` — в каждое состояние роли. Переходы идут между соседями, поэтому
  Combat не перезапускается и задача стрельбы не отдаёт токен при смене роли. Полосу и сектор Keep Distance всё равно
  берёт из `ApplyCoordination`, а состояния ролей — это место для поведения, которое будет у ролей своим.
  Условия лежат в [EnemyStateTreeConditions.h](../../Source/TypicalShooter/Public/Enemies/EnemyStateTreeConditions.h).

## Настройка контента
Project Settings → Game → **Enemy Coordination**:
- **Groups:** `Update Interval` (0.5 с).
- **Tokens:** `Token Fraction` (0.4), `Min Tokens` (1), `Max Tokens` (3); `Token Rotation` — `Token Hold Time` (3 с),
  `Lost Sight Grace` (0.5 с), `Token Cooldown` (1.5 с).
- **Roles:** `Role Weights` — `Pressure` 2, `Flank` 1, `Cover` 1; `Role Bands` — `Pressure` 500–900, `Flank` 700–1200,
  `Cover` 1400–2000.
- **Sectors:** `Flank View Half Angle` (60°), `Flank Reaim Degrees` (30°), `Min Separation` (250 см).

Класс врага (`BP_Enemy` и будущие типы), категория **Enemy | Coordination**:
- `Allowed Roles` — какие роли можно выдавать. Пустой список = любые.
- `Role Band Overrides` — свои полосы по ролям. Например, ближний бой: `Pressure` 0–150, `Flank` 0–200.

Ближний бой и камикадзе в C++ переопределяют `CanAttack` (своя дальность атаки) и решают, что делать с токеном (удар,
рывок). Координатор для них не меняется.

## Тесты
[EnemiesCoordinationTokenCountTest.cpp](../../Source/TypicalShooter/Private/Tests/EnemiesCoordinationTokenCountTest.cpp):
`TypicalShooter.Enemies.Coordination.TokenCount` — округление, ограничения, пустая группа, минимум не больше размера группы,
значения по умолчанию в настройках проекта.

[EnemiesCoordinationRolesTest.cpp](../../Source/TypicalShooter/Private/Tests/EnemiesCoordinationRolesTest.cpp):
`TypicalShooter.Enemies.Coordination.Roles.Quotas` (метод наибольшего остатка, нулевые веса, умолчания проекта),
`.RespectsWeights`, `.OnlyAllowed` (бойцы ближнего боя давят и фланкуют, снайпер прикрывает; если всем разрешено только
давить, квота перебирается), `.NoOvershootWhenAnotherCanCover` (гибкий враг уступает роль), `.StableOnRecompute`
(никто не меняет роль без причины; новичок занимает свободное место; запрещённая роль сбрасывается),
`.MinimalChangeOnDeath`.

[EnemiesCoordinationSectorsTest.cpp](../../Source/TypicalShooter/Private/Tests/EnemiesCoordinationSectorsTest.cpp):
`TypicalShooter.Enemies.Coordination.Sectors.EvenSpread` (куча расходится на четверти), `.NearestFree` (ровная группа
стоит, почти ровная сдвигается меньше чем на 25°), `.FlankersOutOfView` (фланкёр перед игроком уходит из конуса; одиночный —
за спину; двое из трёх — оба вне конуса), `.ReaimHysteresis` (порог, переход через ±180°, `YawAround`, умолчания).

[EnemiesCoordinationTokensTest.cpp](../../Source/TypicalShooter/Private/Tests/EnemiesCoordinationTokensTest.cpp):
`TypicalShooter.Enemies.Coordination.Tokens.OnlyVisible` (плюс сокращение числа токенов и уход держателя), `.Priority`,
`.RotationAfterHoldTime` (ротация по времени ожидания; одиночка держит токен дольше лимита), `.LostSightGrace`,
`.CooldownAfterRelease` (прикрывающий получает токен раньше давящего на кулдауне; одиночку кулдаун не блокирует; умолчания).

[EnemiesPositioningSectorTest.cpp](../../Source/TypicalShooter/Private/Tests/EnemiesPositioningSectorTest.cpp):
`TypicalShooter.Enemies.Positioning.SectorAngleMovesIntoSector` (враг в полосе, но вне сектора, переходит в сектор),
`.SectorAngleStaysInside` (внутри стоит; слишком близко — двигается; без видимости — ближний край, заход в пределах сектора),
`.Separation` (группы из 4, 6 и 12 врагов, 30 сидов: ближайшая пара точек ≥ 250 см).

[EnemiesDirectorGroupsTest.cpp](../../Source/TypicalShooter/Private/Tests/EnemiesDirectorGroupsTest.cpp) (мир с полом,
настоящие враги и игроки): `TypicalShooter.Enemies.Director.GroupsTwoTargets` (две группы, роли 1 : 1 : 1 и одиночка-давящий,
у каждой группы свои токены), `.GroupsSpreadAndShareTokens` (куча из четырёх получает четверти; токенов —
`GetTokenCount(4)`; отданный токен уходит другому при следующем пересчёте), `.RecomputeOnDeath` (убит держатель токена:
группа, роли, токены и ширина секторов пересчитаны сразу), `.FollowsTargetChange` (ближе появился другой игрок — враг уходит
в его группу на следующем тике, а не через `UpdateInterval`).

[EnemiesCoordinationRoleStandTest.cpp](../../Source/TypicalShooter/Private/Tests/EnemiesCoordinationRoleStandTest.cpp):
`TypicalShooter.Enemies.Coordination.RoleStand.PressureClosesInWithoutToken`, `.CoverHoldsWhileVisible` (вне сектора, но в
полосе и с видимостью — стоит), `.FlankGoesToSector` (плюс полосы по умолчанию).

[EnemiesControllerCoordinationTest.cpp](../../Source/TypicalShooter/Private/Tests/EnemiesControllerCoordinationTest.cpp):
`TypicalShooter.Enemies.Controller.FiresOnlyWithToken` (двое видят игрока, стреляет один; `StopCombat` отдаёт токен, и он
переходит ко второму), `.RoleBand` (враг только с `Cover` получает эту роль и встаёт на середину её полосы),
`.RoleBandOverride` (враг ближнего боя: полоса 0–150 → точка в 75 см от игрока; без переопределения — полоса проекта).

[EnemiesBehaviorRoleStatesTest.cpp](../../Source/TypicalShooter/Private/Tests/EnemiesBehaviorRoleStatesTest.cpp):
`TypicalShooter.Enemies.Behavior.RoleStates` — сгенерированное дерево в мире без PIE. Без игрока активно `Free`, с игроком —
`Pressure` под `Combat`. После `AllowedRoles = {Cover}` дерево переходит в `Cover`, и враг сохраняет токен (то есть Combat не
перезапущен; если направить переходы в сам `Combat`, тест падает). `TypicalShooter.Enemies.Behavior.TreeCompiles` теперь
ждёт 6 состояний.

Обновлены под роли: `TypicalShooter.Enemies.Controller.KeepsDistance` (одиночка — давящий с токеном, полоса 500–900) и
`TypicalShooter.Enemies.Flow` (полоса своей роли ±150; отход — до ближнего края полосы роли − 150).

## Ограничения и грабли

## Открытые задачи
- [ ] Укрытия / EQS-точки вместо круга вокруг цели.
- [ ] Распределение врагов между несколькими игроками (балансировка групп).

---
_Сверено с кодом: 2026-11-26_
