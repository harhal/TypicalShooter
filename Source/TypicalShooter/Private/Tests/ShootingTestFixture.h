// Shared fixture for shooting tests: a bare game world (no PIE).

#pragma once

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

namespace ShootingTest
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** Temporary game world, destroyed with the fixture. */
	struct FWorldFixture
	{
		UWorld* World = nullptr;

		FWorldFixture()
		{
			World = UWorld::CreateWorld(EWorldType::Game, false);
			FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
		}

		~FWorldFixture()
		{
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}

		/** Spawns a plain actor with a registered component of type T. */
		template <typename T>
		T* SpawnWithComponent(const TFunction<void(T&)>& Configure = nullptr) const
		{
			AActor* Actor = World->SpawnActor<AActor>();
			T* Component = NewObject<T>(Actor);
			if (Configure)
			{
				Configure(*Component);
			}
			Component->RegisterComponent();
			return Component;
		}
	};
}

#endif
