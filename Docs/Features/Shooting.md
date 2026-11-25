# Стрельба: абилки основной и альтернативной атаки

**Статус:** 🟡 В работе (ветка `feature/shooting-abilities`): игрок стреляет (ЛКМ / ПКМ / RT / LT), перезаряжается из запаса патронов (R / B), снаряды бьют манекены; предметы в инвентаре меняют атаки (GE, замена паттерна).

## Что это
Шутерная составляющая. Основная и альтернативная атаки — это GAS-абилки, которые запускаются
кнопками атаки. У каждой атаки свой набор параметров (дальность, обойма, очередь, урон по типам и т.д.).
Параметры не зашиты в оружие: они складываются из всего снаряжения в инвентаре игрока и его
взаиморасположения — предметы применяют GameplayEffect'ы, которые меняют атрибуты одной или другой атаки.

Первая версия — **снаряды (projectile)**. Позже добавятся хитскан и коллайдер (зона), а также
GameplayEffect'ы на цель / на себя при попадании.

## Требования

**Параметры атаки**
- R1. У персонажа два независимых набора атрибутов атаки — **основной** и **альтернативной** — с одинаковым
  составом. Изменение атрибута одной атаки не влияет на другую.
- R2. Числовые параметры — GAS-атрибуты набора атаки:
  дистанция начала огня, дистанция конца огня, патронов в обойме (текущее), размер обоймы,
  масштаб паттерна выстрела, скорость снаряда, выстрелов в очереди, интервал между выстрелами
  в очереди, интервал между очередями, время перезарядки, урон за попадание по каждому типу
  (физ., кислота, электричество, плазма, лёд, радиация), коэффициент гравитации, потеря урона при столкновении.
- R3. Значения атрибутов клампятся: патронов в обойме ∈ [0, размер обоймы]; счётчики (обойма,
  выстрелы в очереди) ≥ 1 и целые; интервалы, время, дистанции, урон, масштаб паттерна ≥ 0;
  потеря урона ∈ [0, 1]; скорость снаряда ≥ 1; конец огня ≥ начала огня; гравитация не ограничена.
- R4. Базовые значения задаются стартовым GameplayEffect'ом персонажа; предметы меняют их своими GE (R16).

**Выстрел и снаряды**
- R5. **Паттерн выстрела** — данные атаки (не атрибут): список угловых смещений снарядов относительно
  направления прицела. Число снарядов в выстреле = число точек паттерна; угловые смещения умножаются на
  масштаб паттерна. Предмет может **заменить паттерн атаки целиком** (R16); если таких предметов
  несколько — пока действует последний добавленный (порядок решит фича соседства). Один выстрел тратит 1 патрон из обоймы, сколько бы снарядов он ни выпустил.
- R6. Дистанции считаются **от дула**. Направление выстрела — от дула к точке под прицелом (трасса из камеры).
  Снаряд появляется на этом направлении на **дистанции начала огня** от дула и уничтожается, пролетев путь
  до **дистанции конца огня** от дула. Дуло — компонент-точка `Muzzle` на руках от первого лица.
- R7. Гравитация снаряда = коэффициент гравитации (0 — летит прямо).
- R8. При попадании в объект с получателем урона (R11) снаряд наносит ему текущий урон по всем типам.
- R9. При каждом столкновении (с целью или препятствием) оставшийся урон снаряда умножается на
  `1 − потеря урона`; снаряд летит дальше сквозь объект, пока у него остаётся урон > 0. Потеря `1` = снаряд
  гаснет на первом же столкновении (значение по умолчанию).

**Очереди и перезарядка**
- R10. Нажатие кнопки атаки выпускает очередь: «выстрелов в очереди» выстрелов с «интервалом между выстрелами».
  Следующая очередь возможна не раньше чем через «интервал между очередями» после конца предыдущей;
  при удержании кнопки очереди повторяются автоматически.
- R11. Очередь обрывается, когда кончаются патроны. Выстрел с пустой обоймой запускает перезарядку.
- R12. Перезарядка длится «время перезарядки», во время неё эта атака не стреляет; по окончании обойма
  заполняется из запаса боеприпасов (R15). Ручная перезарядка — отдельная кнопка.

**Получение урона**
- R13. Урон принимается через интерфейс получателя урона на **компоненте** актора. Две реализации:
  - GAS-получатель: применяет к ASC владельца GameplayEffect урона (SetByCaller по тегу типа урона),
    уменьшающий атрибут здоровья;
  - простой получатель без GAS (простреливаемые стены, ящики): своё число прочности, событие при нуле.
- R13a. **Мишень-манекен** (`ATargetDummy`) для проверки стрельбы: GAS-здоровье, над ним надпись
  «здоровье / максимум», каждое попадание пишется в лог; манекен не умирает — при нуле здоровье сразу
  восстанавливается до максимума, растёт счётчик «поражений».
- R14. Урон и все решения о попадании считаются **на сервере**; снаряды спавнятся на сервере и реплицируются.

**Боеприпасы и снаряжение**
- R15. Виды боеприпасов: пули, батареи, гранаты, радиаторы, частицы (теги `Ammo.Bullet`, `Ammo.Battery`,
  `Ammo.Grenade`, `Ammo.Radiator`, `Ammo.Particle`). Запас каждого вида — GAS-атрибут персонажа (набор
  атрибутов боеприпасов, по атрибуту на вид), в инвентаре патроны не хранятся. У атаки есть основной вид
  боеприпасов; перезарядка переносит патроны из его запаса в обойму (сколько влезет / сколько есть);
  пустой запас — перезарядка не начинается.
- R15a. **Модификатор выстрела с требованием боеприпаса**: предмет может добавить к атаке модификатор
  «вид боеприпаса + расход на выстрел + эффект на выстрел». При каждом выстреле: если в запасе этого вида
  хватает — расход списывается и эффект действует на этот выстрел; эффект — только прибавки к параметрам
  выстрела (урон по типам, гравитация, потеря урона и т.п.);
  не хватает — модификатор для этого выстрела отключается, сам выстрел происходит как обычно.
- R16. Предмет снаряжения в инвентаре персонажа может нести GE-модификатор атаки; пока предмет лежит в
  инвентаре, GE применён к персонажу и меняет атрибуты основной или альтернативной атаки; убрали предмет —
  GE снят.

**Управление**
- R17. ЛКМ — основная атака, ПКМ — альтернативная, R — перезарядка; геймпад: RT / LT, перезарядка — B
  (вне экрана инвентаря). Пока экран инвентаря открыт, атаки не запускаются.

**Вне рамок:** хитскан и коллайдер-атаки; GE на цель / на себя при попадании; сопротивления урону;
анимации, звуки, VFX, отдача; предсказание снарядов на клиенте; урон шаблонному `ShooterNPC` и шаблонное
оружие `AShooterWeapon` (не используем); подбор патронов в мире; правило «какую атаку и как меняет
предмет» по соседству в инвентаре — **отдельная большая фича** (здесь R16 берёт атаку из фрагмента предмета).

**Решено:** попадание в цель тоже уменьшает урон и пропускает снаряд дальше (R9); запас патронов —
атрибут, не предметы (R15); влияние предметов по соседству — отдельная фича; паттерн выстрела задаёт
число снарядов (R5); виды боеприпасов и модификаторы с требованием боеприпаса (R15, R15a); управление
R17 подтверждено; дистанции от дула, дуло — `Muzzle` (R6); предмет заменяет паттерн целиком (R5);
эффект на выстрел — только параметры (R15a).

**Открытые вопросы:** нет.

## План итераций
- [x] 1. Теги типов урона + интерфейс получателя урона + `FShotDamage` (урон по тегам типов) + простой получатель с прочностью —
  тест: `TypicalShooter.Shooting.Damage.SimpleReceiver`
- [x] 2. Атрибут здоровья + GE урона (SetByCaller) + GAS-получатель —
  тест: `TypicalShooter.Shooting.Damage.GasReceiver`, `.HealthClamp`
- [x] 3. Мишень-манекен: GAS-здоровье, надпись со здоровьем, восстановление при нуле (R13a) —
  тест: `TypicalShooter.Shooting.TargetDummy.*`
- [x] 4. Актор снаряда: гравитация, предел дистанции, урон, потеря урона и пробитие (R7–R9) —
  тест: `TypicalShooter.Shooting.Projectile.*` (игровой мир без PIE, снаряды по манекенам)
- [x] 5. Наборы атрибутов основной/альтернативной атаки + клампы (R1–R3) —
  тест: `TypicalShooter.Shooting.Attributes.*`
- [x] 5a. Математика выстрела: направления снарядов по паттерну и масштабу, направление от дула к точке
  прицела, точка появления (R5, R6) —
  тест: `TypicalShooter.Shooting.Pattern.*`
- [x] 6. Абилка атаки (слот основная/альтернативная): трата патрона, спавн снарядов, очередь и интервалы (R10, R11) —
  тест: `TypicalShooter.Shooting.Ability.Burst`
- [x] 7. Абилка перезарядки с бесконечным запасом + автоперезарядка на пустой обойме (R11, R12) —
  тест: `TypicalShooter.Shooting.Ability.Reload`
- [x] 8. Подключение к персонажу: выдача абилок и стартового GE, ввод ЛКМ/ПКМ/R + геймпад, блокировка при
  открытом инвентаре (R4, R17) — тест: `TypicalShooter.Shooting.Flow` (PIE, стрельба по мишени)
- [x] 9. Запас боеприпасов: набор атрибутов по видам, вид у атаки, перезарядка тратит запас (R15) —
  тест: `TypicalShooter.Shooting.Ability.ReloadFromReserve`
- [x] 10. Модификаторы атак от предметов в инвентаре: GE на атрибуты + замена паттерна (R5, R16) — тест: `TypicalShooter.Shooting.ItemModifiers`
- [x] 11. Модификаторы выстрела с требованием боеприпаса: списание за выстрел, отключение при нехватке (R15a) —
  тест: `TypicalShooter.Shooting.Ability.AmmoGatedModifier`
- [ ] 12. Контент: мишень на уровне, меш снаряда, счётчик патронов в HUD, тестовые предметы — ручная проверка

## Как устроено
- Теги типов урона `Damage.Type.{Physical,Acid,Electric,Plasma,Ice,Radiation}` — нативные, в
  [ShootingTags.h](../../Source/TypicalShooter/Public/Shooting/ShootingTags.h).
- [`FShotDamage`](../../Source/TypicalShooter/Public/Shooting/ShotDamage.h) — урон одного попадания: `Amounts`
  (тег типа → величина) + `Instigator`; `GetTotal`, `HasDamage`, `Scale` (для потери урона при пробитии).
- [`IShotDamageReceiver`](../../Source/TypicalShooter/Public/Shooting/ShotDamageReceiver.h) — C++-интерфейс
  на **компоненте** актора; `FindOnActor` ищет первый такой компонент. Урон применяется только на сервере.
- [`USimpleDamageReceiverComponent`](../../Source/TypicalShooter/Public/Shooting/SimpleDamageReceiverComponent.h) —
  получатель без GAS: прочность `MaxDurability` (выставляется в `InitializeComponent`), все типы урона
  отнимаются суммой; при нуле один раз `OnDepleted` (BP) / `GetDepletedDelegate` (C++), опционально
  `bDestroyOwnerOnDepleted`.
- GAS-урон: [`UHealthAttributeSet`](../../Source/TypicalShooter/Public/Shooting/HealthAttributeSet.h) —
  `Health` / `MaxHealth` (реплицируются, `Health` клампится в [0, Max]) и мета-атрибут `IncomingDamage`.
  [`UShotDamageEffect`](../../Source/TypicalShooter/Public/Shooting/ShotDamageEffect.h) — мгновенный GE, по
  модификатору на каждый тип урона: SetByCaller с тегом `Damage.Type.*` → `IncomingDamage`; в
  `PostGameplayEffectExecute` сумма вычитается из `Health` и `IncomingDamage` обнуляется (сюда же лягут
  сопротивления). [`UGasDamageReceiverComponent`](../../Source/TypicalShooter/Public/Shooting/GasDamageReceiverComponent.h)
  находит ASC владельца, проставляет все типы (отсутствующие = 0) и применяет `DamageEffect` к нему; в контекст
  эффекта кладутся инстигатор и хит.
- [`ATargetDummy`](../../Source/TypicalShooter/Public/Shooting/TargetDummy.h) — манекен: капсула коллизии
  (`BlockAllDynamic`, блокирует и снаряды, и трассы прицела), цилиндр-меш без коллизии, `UTS_GAS` +
  `UHealthAttributeSet` + `UGasDamageReceiverComponent`, надпись `UTextRenderComponent` «здоровье / максимум»
  (обновляется по изменению атрибута, в т.ч. на клиентах). Сервер: считает `TotalDamageTaken`, пишет каждое
  попадание в `LogTypicalShooter`, при нуле `++Defeats` и на следующем тике восстанавливает здоровье
  (не внутри исполнения эффекта).
- [`AShotProjectile`](../../Source/TypicalShooter/Public/Shooting/ShotProjectile.h) — снаряд. Создаётся на сервере
  `AShotProjectile::SpawnShot(World, Class, Transform, FProjectileShotParams, Owner, Instigator)` и летит вдоль
  оси X трансформа. `FProjectileShotParams`: урон (`FShotDamage`), скорость, коэффициент гравитации, длина пути
  `MaxTravelDistance` (абилка передаст «конец огня − начало огня»), потеря урона за попадание. Коллизия — сфера
  с профилем `Projectile`, игнорирует другие снаряды, трассы `Visibility` / `Camera`, владельца и инстигатора.
  Пройденный путь считается в `Tick` (`TG_PostPhysics`); дошёл до предела — уничтожается.
- Пробитие — [`UShotProjectileMovementComponent`](../../Source/TypicalShooter/Public/Shooting/ShotProjectileMovementComponent.h):
  на каждом блокирующем хите вызывает `AShotProjectile::HandleShotHit` (урон получателю на сервере, затем
  `Damage *= 1 − потеря`). Если урон остался и потеря < 1 — пробитый актор игнорируется до конца полёта и
  движение продолжается (`AdvanceNextSubstep`), иначе обычная остановка → уничтожение.
- Параметры атак — [`UAttackAttributeSet`](../../Source/TypicalShooter/Public/Shooting/AttackAttributeSet.h): один
  набор на оба слота, атрибуты `Primary_<Параметр>` / `Alternate_<Параметр>` (GAS не различает два экземпляра одного
  класса набора). Параметры — `EAttackAttribute` в [AttackTypes.h](../../Source/TypicalShooter/Public/Shooting/AttackTypes.h);
  атрибут ищется по имени свойства: `GetAttribute(Slot, Attribute)`. Клампы R3 — `ClampValue` в
  `PreAttributeChange` / `PreAttributeBaseChange`; уменьшение обоймы срезает патроны (`PostAttributeChange`).
  `ReadStats(Asc, Slot)` → `FAttackStats` (всё, с чем работает абилка; конец огня ≥ начала, урон по тегам).
  Значения по умолчанию: обойма 1, очередь 1, масштаб паттерна 1, скорость 5000, конец огня 10000, потеря 1.
  Реплицируются все атрибуты; события изменения на клиентах — только у патронов и размера обоймы (HUD).
- Математика выстрела — [ShotPattern.h](../../Source/TypicalShooter/Public/Shooting/ShotPattern.h): `FShotPattern` —
  угловые смещения (X = рыскание вправо, Y = тангаж вверх, градусы), по снаряду на точку (пусто = один по центру).
  `ShotMath::AimDirection` (дуло → точка прицела, запасное направление при совпадении), `ProjectileDirections`
  (смещение × масштаб паттерна в локальной системе прицела), `SpawnTransform` (точка на дистанции начала огня от дула).
- Прицел — [`FShotAim`](../../Source/TypicalShooter/Public/Shooting/ShotAim.h): трасса `Visibility` из глаз аватара
  (`GetActorEyesViewPoint`; у персонажа на сервере — реплицированный ControlRotation) на дистанцию конца огня;
  дуло — компонент аватара с тегом `Muzzle`, иначе глаза.
- Атака — [`UAttackGameplayAbility`](../../Source/TypicalShooter/Public/Shooting/AttackGameplayAbility.h) (`Slot`, `Pattern`,
  `ProjectileClass`): `InstancedPerActor`, `ServerOnly`, `bReplicateInputDirectly` (сервер видит зажатую кнопку).
  Активация → очередь: каждый выстрел читает `ReadStats`, тратит 1 патрон и спавнит по снаряду на точку паттерна
  (путь снаряда = конец − начало огня); выстрелы с нулевым интервалом — в том же кадре. После очереди абилка
  ждёт интервал между очередями (не реже раза в тик) и, если кнопка зажата, стреляет снова, иначе завершается —
  поэтому новое нажатие раньше интервала ничего не делает. Пустая обойма обрывает очередь, завершает абилку и
  запускает перезарядку слота (`UReloadGameplayAbility::TryReload`); пока слот перезаряжается, атака не активируется.
  Таймеры — `FTimerManager` мира, сбрасываются в `EndAbility`.
- Перезарядка — [`UReloadGameplayAbility`](../../Source/TypicalShooter/Public/Shooting/ReloadGameplayAbility.h) (`Slot`,
  `ServerOnly`): полная обойма — сразу завершается; иначе вешает loose-тег `State.Reloading.<Слот>` (блокирует атаку
  слота в её `CanActivateAbility`; реплицируется для HUD), ждёт время перезарядки и переносит в обойму `min(не хватает, запас)` патронов
  вида `AmmoType` (пустой тег — бесконечный запас). Нет запаса — перезарядка не начинается.
- Запас — [`UAmmoAttributeSet`](../../Source/TypicalShooter/Public/Shooting/AmmoAttributeSet.h): `Reserve_Bullet / Battery /
  Grenade / Radiator / Particle` (целые, ≥ 0, реплицируются с уведомлением для HUD); `GetReserveAttribute(Ammo.*)`.
  Теги видов — `Ammo.*` в ShootingTags.h.
- Предметы → атаки — фрагмент [`FAttackModifier_ItemFragment`](../../Source/TypicalShooter/Public/Shooting/AttackModifierFragment.h)
  (`Effect` — GE, обычно бесконечный, на атрибуты `Primary_*` / `Alternate_*` или запасы; `bReplacePattern` +
  `PatternSlot` + `Pattern`) и [`UAttackModifiersComponent`](../../Source/TypicalShooter/Public/Shooting/AttackModifiersComponent.h)
  на персонаже. Компонент (сервер) слушает добавление / удаление предметов своего `UInventoryComponent`: применяет
  GE предмета к ASC владельца (источник — предмет) и снимает его вместе с предметом; модификаторы складываются.
  Замены паттерна хранятся в порядке прихода, действует последняя для слота (`FindPatternOverride`); атака берёт её
  вместо своего `Pattern`. Перемещение внутри инвентаря ничего не меняет (расположение — будущая фича соседства).
  Персонаж вызывает `InitAbilityActorInfo` до `Super::BeginPlay`, чтобы компонент мог применить эффекты сразу.
- Модификаторы выстрела с требованием боеприпаса — [`FAmmoGatedShotModifier`](../../Source/TypicalShooter/Public/Shooting/ShotModifier.h)
  в `ShotModifiers` того же фрагмента: слот, вид боеприпаса (`Ammo.*`), расход на выстрел, прибавки
  `FAttackStatBonus` (параметр + величина). Атака на каждом выстреле (после траты патрона обоймы) проходит по
  модификаторам слота в порядке прихода предметов: хватает запаса — списывает расход и прибавляет бонусы к копии
  `FAttackStats` этого выстрела (`UAttackAttributeSet::AddToStats`, с клампами; обойма, очередь и патроны не
  меняются), не хватает — модификатор пропускается, выстрел происходит как обычно.
- HUD — [`UAmmoCounterWidget`](../../Source/TypicalShooter/Public/UI/Widget/AmmoCounterWidget.h) (UMG-обёртка над
  двумя `STextBlock`): строки «Primary: обойма / запас» и «Alternate: …», текст из
  [`AmmoDisplay::GetSlotText`](../../Source/TypicalShooter/Public/Shooting/AmmoDisplay.h) каждый кадр по ASC пешки владельца
  (запас — по `AmmoType` перезарядки слота, `∞` если не ограничен; во время перезарядки «Reloading...»). Тег
  `State.Reloading.*` теперь реплицируется (`EGameplayTagReplicationState::TagOnly`), чтобы клиент видел перезарядку.
- Персонаж — [`ATypicalShooterCharacter`](../../Source/TypicalShooter/Public/TypicalShooterCharacter.h): `UTS_GAS`
  (реплицируется, Mixed) + `UAttackAttributeSet`, компонент `Muzzle` (тег `Muzzle`, у камеры: 40 вперёд, 15 вправо,
  12 вниз). На сервере в `BeginPlay` выдаёт `AttackAbilities` (класс + `EAttackInputID` как InputID спека) и
  применяет `AttackDefaultsEffect`. По умолчанию — [DefaultAttackSetup.h](../../Source/TypicalShooter/Public/Shooting/DefaultAttackSetup.h):
  `UPrimaryAttackAbility` («автомат»: обойма 30, очередь 1, интервал 0.12 с при удержании, 10 физ., 8000 см/с),
  `UAlternateAttackAbility` («дробовик»: 7 снарядов — центр + кольцо 3°, обойма 6, интервал 0.8 с, 6 физ. за
  снаряд, 3000 см, гравитация 0.2), `UPrimaryReloadAbility` / `UAlternateReloadAbility` (1.5 / 2 с) и
  `UDefaultAttackStatsEffect` (мгновенный GE с Override; обойма раньше патронов; запас: 120 пуль, 24 частицы).
  Вид боеприпасов: автомат — `Ammo.Bullet`, дробовик — `Ammo.Particle` (в классах перезарядки).
- Ввод — [`UAttackInputComponent`](../../Source/TypicalShooter/Public/Shooting/AttackInputComponent.h) на персонаже,
  действия и контекст генерируются в коде (как у инвентаря): ЛКМ / RT → `PrimaryAttack`, ПКМ / LT →
  `AlternateAttack`, R / B → `Reload` (обе перезарядки). Нажатие → `AbilityLocalInputPressed(InputID)` (активирует
  или отмечает удержание; на клиенте уходит на сервер), отпускание → `AbilityLocalInputReleased`. Пока открыт
  экран инвентаря, нажатия игнорируются (отпускания — нет).

## Настройка контента
Простреливаемая стена / ящик: добавить актору компонент `Simple Damage Receiver`, задать `Max Durability`
и при необходимости `Destroy Owner On Depleted`.

Мишень: перетащить `Target Dummy` (C++-класс) на уровень, при желании поменять `Max Health`.

Предмет, меняющий атаку: в манифест предмета добавить фрагмент `Attack Modifier_Item Fragment`, в `Effect`
указать GE-ассет (Duration = Infinite, модификаторы атрибутов `AttackAttributeSet.Primary_*` / `Alternate_*`), при
желании включить `Replace Pattern` и задать слот и точки паттерна.
Платный бонус к выстрелу: в том же фрагменте `Shot Modifiers` → слот, `Ammo Type`, `Cost Per Shot`, `Bonuses`.

Атаки персонажа: в `BP_FirstPersonCharacter` → `Attack Abilities` (какие абилки и на какие кнопки) и
`Attack Defaults Effect` (базовые значения; можно заменить своим GE-ассетом с Override-модификаторами атрибутов
`AttackAttributeSet.Primary_*` / `Alternate_*`). Паттерн альтернативной атаки — в её классе (`Pattern`).

## Тесты
[ShootingDamageTest.cpp](../../Source/TypicalShooter/Private/Tests/ShootingDamageTest.cpp) (игровой мир без PIE,
фикстура [ShootingTestFixture.h](../../Source/TypicalShooter/Private/Tests/ShootingTestFixture.h)):
`TypicalShooter.Shooting.Damage.ShotDamageAmounts`, `.SimpleReceiver`, `.SimpleReceiverDestroysOwner`.
[ShootingGasDamageTest.cpp](../../Source/TypicalShooter/Private/Tests/ShootingGasDamageTest.cpp): `TypicalShooter.Shooting.Damage.HealthClamp`, `.GasReceiver`.
[ShootingTargetDummyTest.cpp](../../Source/TypicalShooter/Private/Tests/ShootingTargetDummyTest.cpp) (мир с
`SetBegunPlay(true)` и ручным тиком — `FWorldFixture(true)` / `Tick`): `TypicalShooter.Shooting.TargetDummy.TakesDamage`,
`.RefillsWhenDepleted`.
[ShootingProjectileTest.cpp](../../Source/TypicalShooter/Private/Tests/ShootingProjectileTest.cpp) (снаряды по
манекенам и стене в тикающем мире): `TypicalShooter.Shooting.Projectile.DamagesDummy` (и не задевает владельца),
`.PiercesWithDamageLoss`, `.ObstacleReducesDamage`, `.ObstacleStopsWithFullLoss`, `.ExpiresAfterMaxDistance`,
`.GravityScale`.
[ShootingAttributesTest.cpp](../../Source/TypicalShooter/Private/Tests/ShootingAttributesTest.cpp):
`TypicalShooter.Shooting.Attributes.SlotsIndependent`, `.Clamp`, `.AmmoFollowsClipSize`, `.ReadStats`.
[ShootingPatternTest.cpp](../../Source/TypicalShooter/Private/Tests/ShootingPatternTest.cpp):
`TypicalShooter.Shooting.Pattern.SinglePointFollowsAim`, `.OffsetsScaled`, `.AimFromMuzzle`.
[ShootingAbilityTest.cpp](../../Source/TypicalShooter/Private/Tests/ShootingAbilityTest.cpp) (актор с ASC стреляет
по манекену): `TypicalShooter.Shooting.Ability.SingleShot`, `.Burst`, `.HoldRepeatsBursts`, `.BurstStopsWhenEmpty`,
`.PatternProjectiles`, `.AlternateSlot`.
[ShootingReloadTest.cpp](../../Source/TypicalShooter/Private/Tests/ShootingReloadTest.cpp) (общий стрелок —
[ShootingAbilityFixture.h](../../Source/TypicalShooter/Private/Tests/ShootingAbilityFixture.h)):
`TypicalShooter.Shooting.Ability.Reload`, `.ReloadSkipsFullClip`, `.AutoReloadWhenEmpty`, `.EmptyPressReloads`,
`.ReloadFromReserve`, `.NoReloadWithoutReserve`.
[ShootingItemModifiersTest.cpp](../../Source/TypicalShooter/Private/Tests/ShootingItemModifiersTest.cpp) (тестовый GE —
[ShootingTestEffects.h](../../Source/TypicalShooter/Private/Tests/ShootingTestEffects.h)):
`TypicalShooter.Shooting.ItemModifiers.EffectWhileHeld`, `.PatternReplacement`, `.AttackUsesModifiers`.
`TypicalShooter.Shooting.Ability.AmmoGatedModifier` — там же.
[ShootingAmmoDisplayTest.cpp](../../Source/TypicalShooter/Private/Tests/ShootingAmmoDisplayTest.cpp): `TypicalShooter.Shooting.UI.AmmoText`.
[ShootingFlowTest.cpp](../../Source/TypicalShooter/Private/Tests/ShootingFlowTest.cpp) — PIE `TypicalShooter.Shooting.Flow`:
стартовые обоймы 30 / 6, ЛКМ и ПКМ попадают по манекену перед игроком, R перезаряжает обе атаки (и списывает запас), с открытым
инвентарём ЛКМ не стреляет, RT стреляет.

## Ограничения и грабли
- Тестовые GE (`UTestPrimaryDamageBonusEffect`) — UCLASS, поэтому есть во всех сборках (скрыты из выпадающих списков).
- Модификаторы предметов применяются только на сервере; замена паттерна не реплицируется (атака тоже серверная).
- В тестовом мире `World->Tick` несколько раз за один кадр движка тикает акторов только в первый раз
  (защита по `GFrameCounter`) — `FWorldFixture::Tick` увеличивает `GFrameCounter` на каждом шаге.
- `TMap` не реплицируется: урон снаряда (`FShotDamage`) живёт только на сервере, клиентам уходят лишь скорость
  и гравитация (`Flight`, один раз) и реплицируемое движение.
- Пробитие игнорирует пробитого актора целиком до конца полёта: снаряд не может попасть в одну цель дважды.

## Открытые задачи
- [ ] Хитскан-атаки.
- [ ] Атаки-коллайдеры (зона).
- [ ] GameplayEffect на цель / на себя при попадании.
- [ ] Сопротивления типам урона.

---
_Сверено с кодом: 2026-11-25_
