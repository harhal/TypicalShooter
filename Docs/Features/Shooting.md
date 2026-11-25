# Стрельба: абилки основной и альтернативной атаки

**Статус:** ✅ Работает (снаряды): игрок стреляет основной / альтернативной атакой (ЛКМ / ПКМ / RT / LT), снаряды пробивают и бьют манекены, предметы в инвентаре меняют атаки; HUD показывает запас боеприпасов. Обойма и перезарядка убраны из атак — они переезжают в источники [энергосети](ItemPowerGrid.md) (R9a). Мультиплеер не проверялся.

## Что это
Шутерная составляющая. Основная и альтернативная атаки — это GAS-абилки, которые запускаются
кнопками атаки. У каждой атаки свой набор параметров (дальность, очередь, урон по типам и т.д.).
Параметры не зашиты в оружие: они складываются из всего снаряжения в инвентаре игрока и его
взаиморасположения — предметы применяют GameplayEffect'ы, которые меняют атрибуты одной или другой атаки.

Первая версия — **снаряды (projectile)**. Позже добавятся хитскан и коллайдер (зона), а также
GameplayEffect'ы на цель / на себя при попадании.

## Требования

**Параметры атаки**
- R1. У персонажа два независимых набора атрибутов атаки — **основной** и **альтернативной** — с одинаковым
  составом. Изменение атрибута одной атаки не влияет на другую.
- R2. Числовые параметры — GAS-атрибуты набора атаки:
  дистанция начала огня, дистанция конца огня,
  масштаб паттерна выстрела, скорость снаряда, выстрелов в очереди, интервал между выстрелами
  в очереди, интервал между очередями, урон за попадание по каждому типу
  (физ., кислота, электричество, плазма, лёд, радиация), коэффициент гравитации, потеря урона при столкновении,
  отдача ([энергосеть](ItemPowerGrid.md) R5c). Третий набор — **воздушная** атака (`Air_*`, энергосеть R5b).
  (Патроны в обойме, размер обоймы и время перезарядки убраны — [энергосеть](ItemPowerGrid.md) R9a.)
- R3. Значения атрибутов клампятся: выстрелы в очереди ≥ 1 и целые; интервалы, дистанции, урон, масштаб паттерна ≥ 0;
  потеря урона ∈ [0, 1]; скорость снаряда ≥ 1; конец огня ≥ начала огня; гравитация не ограничена.
- R4. Базовые значения слота задаёт работающий в нём приёмник [энергосети](ItemPowerGrid.md) (R5a); без него —
  нейтральные значения набора. Предметы меняют их эффектами по мощности ([энергосеть](ItemPowerGrid.md) R13–R14a).

**Выстрел и снаряды**
- R5. **Паттерн выстрела** — данные атаки (не атрибут): список угловых смещений снарядов относительно
  направления прицела. Число снарядов в выстреле = число точек паттерна; угловые смещения умножаются на
  масштаб паттерна. Паттерн задаёт класс абилки приёмника ([энергосеть](ItemPowerGrid.md) R14b);
  ~~замена паттерна предметом~~ убрана.
- R6. Дистанции считаются **от дула**. Направление выстрела — от дула к точке под прицелом (трасса из камеры).
  Снаряд появляется на этом направлении на **дистанции начала огня** от дула и уничтожается, пролетев путь
  до **дистанции конца огня** от дула. Дуло — компонент-точка `Muzzle` на руках от первого лица.
- R7. Гравитация снаряда = коэффициент гравитации (0 — летит прямо).
- R8. При попадании в объект с получателем урона (R11) снаряд наносит ему текущий урон по всем типам.
- R9. При каждом столкновении (с целью или препятствием) оставшийся урон снаряда умножается на
  `1 − потеря урона`; снаряд летит дальше сквозь объект, пока у него остаётся урон > 0. Потеря `1` = снаряд
  гаснет на первом же столкновении (значение по умолчанию).

**Очереди**
- R10. Нажатие кнопки атаки выпускает очередь: «выстрелов в очереди» выстрелов с «интервалом между выстрелами».
  Следующая очередь возможна не раньше чем через «интервал между очередями» после конца предыдущей;
  при удержании кнопки очереди повторяются автоматически.
- ~~R11. Очередь обрывается, когда кончаются патроны; пустая обойма запускает перезарядку.~~
  ~~R12. Перезарядка из запаса, ручная перезарядка — отдельная кнопка.~~ Убраны: обойма и её пополнение живут
  в источниках [энергосети](ItemPowerGrid.md) (R9, R9a, R9c); у атаки обоймы нет.

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
  атрибутов боеприпасов, по атрибуту на вид), в инвентаре патроны не хранятся. Запас тратят обоймы
  источников [энергосети](ItemPowerGrid.md) (R9–R9d).
- ~~R15a. Модификатор выстрела с требованием боеприпаса.~~ ~~R16. GE-модификатор атаки от предмета.~~ Заменены
  энергосетью (R14b): бонус предмета — эффект по порогу мощности, «платный» бонус — источник с боеприпасами,
  подводящий мощность к предмету-усилителю.

**Управление**
- R17. ЛКМ — основная атака, ПКМ — альтернативная; геймпад: RT / LT (вне экрана инвентаря). Пока экран
  инвентаря открыт, атаки не запускаются. (Кнопка перезарядки R / B убрана вместе с перезарядкой.)

**Вне рамок:** хитскан и коллайдер-атаки; GE на цель / на себя при попадании; сопротивления урону;
анимации, звуки, VFX, отдача; предсказание снарядов на клиенте; урон шаблонному `ShooterNPC` и шаблонное
оружие `AShooterWeapon` (не используем); подбор патронов в мире; правило «какую атаку и как меняет
предмет» по соседству в инвентаре — фича [энергосети](ItemPowerGrid.md).

**Решено:** попадание в цель тоже уменьшает урон и пропускает снаряд дальше (R9); запас патронов —
атрибут, не предметы (R15); влияние предметов по соседству — отдельная фича; паттерн выстрела задаёт
число снарядов (R5); виды боеприпасов и модификаторы с требованием боеприпаса (R15, R15a); управление
R17 подтверждено; дистанции от дула, дуло — `Muzzle` (R6); модификаторы предметов (R5, R15a, R16) заменены энергосетью;
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
- [x] 12. Контент: три манекена на `Lvl_FirstPerson`, счётчик патронов в `BP_HUD` — проверено в PIE (снимок экрана);
  тестовые предметы-модификаторы в контейнерах перенесены в «Открытые задачи»
- [x] 13. (Энергосеть, итерация 7) Удалены атрибуты обоймы и перезарядки, абилки перезарядки и кнопка R / B; HUD
  показывает запас по видам — тесты: `TypicalShooter.Shooting.Ability.NoClipLimit`, `.UI.AmmoText`, `.Flow` (обновлены)
- [x] 14. (Энергосеть, итерация 7a) Слот `Air` (`Air_*`) и атрибут отдачи `Recoil` — тест: `TypicalShooter.Shooting.Attributes.AirSlot`, `.Recoil`
- [x] 15. (Энергосеть, итерация 9) Атаки выдают приёмники в слотах, база — от приёмника; воздушная атака вниз; отдача —
  тесты: `TypicalShooter.PowerGrid.Receivers.*`, `TypicalShooter.Shooting.Ability.Recoil`
- [x] 16. (Энергосеть, итерация 12) Удалены `FAttackModifier_ItemFragment`, `UAttackModifiersComponent` и
  `FAmmoGatedShotModifier`: бонусы предметов — эффекты по мощности, паттерн — абилка приёмника, платный бонус —
  источник с боеприпасами (R5, R15a, R16 сняты) — тесты: `TypicalShooter.PowerGrid.Effects.ShotUsesBonuses`,
  `TypicalShooter.PowerGrid.Receivers.AbilityPattern`

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
  набор на все слоты, атрибуты `Primary_<Параметр>` / `Alternate_<Параметр>` / `Air_<Параметр>` (GAS не различает два экземпляра одного
  класса набора). Параметры — `EAttackAttribute` в [AttackTypes.h](../../Source/TypicalShooter/Public/Shooting/AttackTypes.h);
  атрибут ищется по имени свойства: `GetAttribute(Slot, Attribute)`. Клампы R3 — `ClampValue` в
  `PreAttributeChange` / `PreAttributeBaseChange`.
  `ReadStats(Asc, Slot)` → `FAttackStats` (всё, с чем работает абилка; конец огня ≥ начала, урон по тегам).
  Значения по умолчанию: очередь 1, масштаб паттерна 1, скорость 5000, конец огня 10000, потеря 1.
  Реплицируются все атрибуты (без событий изменения на клиентах).
- Математика выстрела — [ShotPattern.h](../../Source/TypicalShooter/Public/Shooting/ShotPattern.h): `FShotPattern` —
  угловые смещения (X = рыскание вправо, Y = тангаж вверх, градусы), по снаряду на точку (пусто = один по центру).
  `ShotMath::AimDirection` (дуло → точка прицела, запасное направление при совпадении), `ProjectileDirections`
  (смещение × масштаб паттерна в локальной системе прицела), `SpawnTransform` (точка на дистанции начала огня от дула).
- Прицел — [`FShotAim`](../../Source/TypicalShooter/Public/Shooting/ShotAim.h): трасса `Visibility` из глаз аватара
  (`GetActorEyesViewPoint`; у персонажа на сервере — реплицированный ControlRotation) на дистанцию конца огня;
  дуло — компонент аватара с тегом `Muzzle`, иначе глаза.
- Атака — [`UAttackGameplayAbility`](../../Source/TypicalShooter/Public/Shooting/AttackGameplayAbility.h) (`Slot`, `Pattern`,
  `ProjectileClass`): `InstancedPerActor`, `ServerOnly`, `bReplicateInputDirectly` (сервер видит зажатую кнопку).
  Активация → очередь: каждый выстрел читает `ReadStats` и спавнит по снаряду на точку паттерна
  (путь снаряда = конец − начало огня); выстрелы с нулевым интервалом — в том же кадре. После очереди абилка
  ждёт интервал между очередями (не реже раза в тик) и, если кнопка зажата, стреляет снова, иначе завершается —
  поэтому новое нажатие раньше интервала ничего не делает. Обоймы нет: число выстрелов ничем не ограничено
  (ограничивать будут источники энергосети). Таймеры — `FTimerManager` мира, сбрасываются в `EndAbility`.
- Запас — [`UAmmoAttributeSet`](../../Source/TypicalShooter/Public/Shooting/AmmoAttributeSet.h): `Reserve_Bullet / Battery /
  Grenade / Radiator / Particle` (целые, ≥ 0, реплицируются с уведомлением для HUD); `GetReserveAttribute(Ammo.*)`.
  Теги видов — `Ammo.*` в ShootingTags.h, список в фиксированном порядке — `ShootingTags::GetAmmoTypes()`.
- Предметы → атаки — только через [энергосеть](ItemPowerGrid.md): бонусы атрибутов — эффекты по мощности
  (`UPowerEffectsComponent`), паттерн — `Pattern` класса абилки приёмника, расход боеприпасов — обоймы источников
  (`UAttackGameplayAbility::ReportUse` после каждого выстрела). `FAttackStatBonus` (параметр + величина,
  [ShotModifier.h](../../Source/TypicalShooter/Public/Shooting/ShotModifier.h)) — общий формат базы приёмника и бонусов.
  Персонаж вызывает `InitAbilityActorInfo` до `Super::BeginPlay`, чтобы компоненты сети могли применить эффекты сразу.
- HUD — [`UAmmoCounterWidget`](../../Source/TypicalShooter/Public/UI/Widget/AmmoCounterWidget.h) (UMG-обёртка над
  `STextBlock`): запас по видам, строка на вид с ненулевым запасом («Bullet: 120»), текст из
  [`AmmoDisplay::GetReserveText`](../../Source/TypicalShooter/Public/Shooting/AmmoDisplay.h) каждый кадр по ASC пешки владельца.
- Персонаж — [`ATypicalShooterCharacter`](../../Source/TypicalShooter/Public/TypicalShooterCharacter.h): `UTS_GAS`
  (реплицируется, Mixed) + `UAttackAttributeSet`, компонент `Muzzle` (тег `Muzzle`, у камеры: 40 вперёд, 15 вправо,
  12 вниз). Атаки персонаж больше не выдаёт сам: их дают приёмники энергосети в слотах
  (`UReceiverAbilitiesComponent`, [энергосеть](ItemPowerGrid.md) R5, R6a), а базовые параметры атаки — база
  атрибутов слота от приёмника (R5a). На сервере в `BeginPlay` персонаж применяет `AttackDefaultsEffect`
  (`UDefaultAttackStatsEffect`: только запас — 120 пуль, 24 частицы) и кладёт в инвентарь `StartingItems`
  ([DefaultLoadout.h](../../Source/TypicalShooter/Public/PowerGrid/DefaultLoadout.h)): источник в центре, приёмник
  «винтовка» (`UPrimaryAttackAbility`: интервал 0.12 с при удержании, 10 физ., 8000 см/с, 8000 см) в основном слоте,
  «дробовик» (`UAlternateAttackAbility`: 7 снарядов — центр + кольцо 3°, интервал 0.8 с, 6 физ. за снаряд, 6000 см/с,
  3000 см, гравитация 0.2) в альтернативном. Классы абилок — в [DefaultAttackSetup.h](../../Source/TypicalShooter/Public/Shooting/DefaultAttackSetup.h).
- Ввод — [`UAttackInputComponent`](../../Source/TypicalShooter/Public/Shooting/AttackInputComponent.h) на персонаже,
  действия и контекст генерируются в коде (как у инвентаря): ЛКМ / RT → `PrimaryAttack`, ПКМ / LT →
  `AlternateAttack`. Нажатие → `AbilityLocalInputPressed(InputID)` (активирует
  или отмечает удержание; на клиенте уходит на сервер), отпускание → `AbilityLocalInputReleased`. Пока открыт
  экран инвентаря, нажатия игнорируются (отпускания — нет).

## Настройка контента
Простреливаемая стена / ящик: добавить актору компонент `Simple Damage Receiver`, задать `Max Durability`
и при необходимости `Destroy Owner On Depleted`.

Мишень: перетащить `Target Dummy` (C++-класс) на уровень, при желании поменять `Max Health`. На `Lvl_FirstPerson`
стоят три: `TargetDummy_Center` (800, 0), `TargetDummy_Left` / `_Right` (1100, ∓300), лицом к старту игрока.

Счётчик патронов: `BP_HUD` → `AmmoCounter` (`UAmmoCounterWidget`) на корневом Canvas, якорь в правом нижнем углу
(отступ 40), автоматический размер; шрифт и цвет — в его свойствах.

Предмет, меняющий атаку: фрагмент `Power Effects_Item Fragment` (ступени с порогом мощности, `Attack Bonuses`
и/или `Character Effect`) — см. [энергосеть](ItemPowerGrid.md).

Атаки персонажа: в `BP_FirstPersonCharacter` → `Starting Items` (предметы и клетки; атаки дают приёмники с фрагментом
`Power Grid_Item Fragment` в слотах — см. [энергосеть](ItemPowerGrid.md)) и `Attack Defaults Effect` (стартовый запас).
Паттерн атаки — в классе абилки приёмника (`Pattern`).

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
`TypicalShooter.Shooting.Attributes.SlotsIndependent`, `.Clamp`, `.AirSlot`, `.Recoil`, `.ReadStats`.
[ShootingPatternTest.cpp](../../Source/TypicalShooter/Private/Tests/ShootingPatternTest.cpp):
`TypicalShooter.Shooting.Pattern.SinglePointFollowsAim`, `.OffsetsScaled`, `.AimFromMuzzle`.
[ShootingAbilityTest.cpp](../../Source/TypicalShooter/Private/Tests/ShootingAbilityTest.cpp) (актор с ASC стреляет
по манекену; общий стрелок — [ShootingAbilityFixture.h](../../Source/TypicalShooter/Private/Tests/ShootingAbilityFixture.h),
выстрелы считаются по заспавненным снарядам): `TypicalShooter.Shooting.Ability.SingleShot`, `.Burst`,
`.HoldRepeatsBursts`, `.NoClipLimit`, `.PatternProjectiles`, `.AlternateSlot`.
Влияние предметов на выстрел — тесты энергосети (`PowerGrid.Effects.ShotUsesBonuses`, `PowerGrid.Receivers.AbilityPattern`).
[ShootingAmmoDisplayTest.cpp](../../Source/TypicalShooter/Private/Tests/ShootingAmmoDisplayTest.cpp): `TypicalShooter.Shooting.UI.AmmoText`.
[ShootingFlowTest.cpp](../../Source/TypicalShooter/Private/Tests/ShootingFlowTest.cpp) — PIE `TypicalShooter.Shooting.Flow`:
стартовый урон 10 / 6, ЛКМ (1 снаряд) и ПКМ (7 снарядов) попадают по манекену перед игроком, R ничего не делает, с
открытым инвентарём ЛКМ не стреляет, RT стреляет.

## Ограничения и грабли
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
- [ ] Свой меш / эффект снаряда (сейчас серая сфера 10 см из `/Engine/BasicShapes`).
- [ ] Проверить стрельбу по сети (клиент → сервер, репликация снарядов).

---
_Сверено с кодом: 2026-11-25_
