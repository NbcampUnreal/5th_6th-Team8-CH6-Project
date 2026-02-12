#pragma once

#include "NativeGameplayTags.h"

namespace GameplayTags
{
	namespace AbilityBlockTag
	{
		PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Fire);
	}
	
	namespace Ability
	{
		namespace CoolDown
		{
			PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Banana);
		}
	}
	

	namespace Data
	{
		PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Damage);
		PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Duration);
	}
	
	namespace Event
	{
		PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Bomb_Explode);
		PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Gimmick_Collect);
		PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_ReloadReFill);
		namespace Round
		{
			PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Economy_Depression);
			PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Economy_Inflation);
		}
	}
	
	namespace MiniGame
	{
		PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Bomb);
	}
	
	namespace Input
	{
		PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Crouch);
		PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equip);
		PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Fire);
		PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Jump);
		PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Reload);
		PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sprint);
	}
	
	namespace State
	{
		PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(HitReaction_HeadShot);
		PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Posture_Crouching);
		PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Dead);
		PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Bomb);
		PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Invincible);
		PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stasis);
		PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Slowing);
		
		namespace Passive
		{
			PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(ReflectShield);
		}
		
		namespace Movement
		{
			PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InAir);
			PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sprinting);
		}
		
	}
	namespace Item
	{
		namespace Passive
		{
			PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(ReflectShield);
		}
	}
	
	namespace Weapon
	{
		namespace Anim
		{
			PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Fire);
			PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Reload);
		}
		namespace Gun
		{
			namespace Pistol
			{
				PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pistol);
				PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(BombPistol);
			}
			PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Rifle);
			PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Rocket);
			PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(WaterGun);
		}
		namespace State
		{
			PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equip);
			PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Reload);
			PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UnEquip);
		}

		namespace EquipType
		{
			PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Basic);
			PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Special);
		}
	}
	
	namespace GameplayCue
	{
		namespace Weapon
		{
			PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Explosion);
			PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Fire);
			PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(HitImpact);
			PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Water);
		}
		
		namespace Item
		{
			PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Invisibility);
			PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Invincible);
		}
		
		namespace Hit
		{
			PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Wall);
		}
	}

	namespace ChaosEvent
	{
		namespace Buff
		{
			PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MoveSpeed);
		}
		namespace Debuff
		{
			PTW_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MoveSpeed);
		}
	}
	
}

