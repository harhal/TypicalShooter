#include "Shooting/ShotDamageReceiver.h"

#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"

IShotDamageReceiver* IShotDamageReceiver::FindOnActor(const AActor* Actor)
{
	if (!Actor)
	{
		return nullptr;
	}
	for (UActorComponent* Component : Actor->GetComponents())
	{
		if (IShotDamageReceiver* Receiver = Cast<IShotDamageReceiver>(Component))
		{
			return Receiver;
		}
	}
	return nullptr;
}
