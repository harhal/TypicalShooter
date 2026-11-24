# TypicalShooter

Roguelike arena shooter on Unreal Engine 5.8 (C++). Its distinguishing feature is the inventory:
items are placed on a hexagonal grid.

## Documentation

- **[Features](Docs/Features/README.md)** — living documentation of game features: what works,
  how it is built and tested, what is left (in Russian).
- **[CLAUDE.md](CLAUDE.md)** — project guide:
  - [Engine & build](CLAUDE.md#engine--build)
  - [Source layout](CLAUDE.md#source-layout)
  - [Conventions](CLAUDE.md#conventions)
  - [Feature documentation](CLAUDE.md#feature-documentation)
  - [Testing (TDD)](CLAUDE.md#testing-tdd)
  - [Git](CLAUDE.md#git)
  - [Architecture](CLAUDE.md#architecture-custom-code)

## Getting started

The hex-grid plugin is a git submodule
([HarhalHexagonalGrid](https://github.com/harhal/HarhalHexagonalGrid)), so clone recursively:

```powershell
git clone --recursive https://github.com/harhal/TypicalShooter.git
```

Binary assets are stored in Git LFS (`git lfs install` before cloning).
