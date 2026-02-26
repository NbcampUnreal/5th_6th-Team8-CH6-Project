// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWWeaponSoundTable.h"

USoundCue* UPTWWeaponSoundTable::GetSoundForTag(FGameplayTag Container) const
{
	for (const auto& Entry : SoundTable)
	{
		if (Container.MatchesTag(Entry.WeaponTag)) return Entry.Sound;
	}
	return nullptr;
}
