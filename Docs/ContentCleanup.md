# Кандидаты на удаление

Контент и код, которые игра больше не использует. Удаляем, когда станет ясно, что они не пригодятся
(или когда мешают). Перед удалением — проверить ссылки (Reference Viewer / поиск пути пакета в `Content/`).

## После удаления шаблонных NPC (фича [врагов](Features/Enemies.md), 2026-11-26)

Удалены вместе с C++-классами `Variant_Shooter/AI` (без них не загружаются): `BP_ShooterNPC`,
`BP_ShooterAIController`, `BP_ShooterNPCSpawner`, `ST_Shooter`, `ST_Shooter_ShootAtTarget`,
`EQS_FindRoamLocation`, `EQS_FindSnipingLocation` и два спавнера NPC на `Lvl_Shooter`.

Остались без пользователя в игре (только шаблонный уровень `Lvl_Shooter`, который не является картой игры):

| Что | Почему кандидат |
|-----|-----------------|
| `Content/Variant_Shooter/Lvl_Shooter` (+ его `__ExternalActors__` / `__ExternalObjects__`) | Шаблонная арена без врагов; игра стартует на `Lvl_FirstPerson` |
| `Content/Variant_Shooter/Blueprints/BP_ShooterGameMode`, `BP_ShooterCharacter`, `BP_ShooterPlayerController` | Используются только `Lvl_Shooter` |
| `Content/Variant_Shooter/Blueprints/Pickups/*` (оружие, снаряды, `DT_WeaponData`) | Шаблонное оружие `AShooterWeapon`; наша стрельба — GAS-атаки. Пистолет раньше держал NPC |
| `Content/Variant_Shooter/Anims/*` (`ABP_TP_Rifle`, `ABP_TP_Pistol`, `ABP_FP_*`, `FP_Rifle_Shoot_Montage`, `Ctrl_HandAdjusment*`) | Анимации шаблонного оружия; `ABP_TP_*` были для NPC |
| `Content/Variant_Shooter/Input/*`, `Content/Variant_Shooter/UI/*` | Ввод и UI шаблонного оружия |
| `Source/TypicalShooter/Variant_Shooter/` (`ShooterCharacter`, `ShooterGameMode`, `ShooterPlayerController`, `Weapons/`, `UI/`) | C++ за этим контентом; удалять вместе с ним |
