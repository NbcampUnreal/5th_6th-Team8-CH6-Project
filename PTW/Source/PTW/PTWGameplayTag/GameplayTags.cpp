#include "GameplayTags.h"

namespace GameplayTags
{
	namespace AbilityBlockTag
	{
		UE_DEFINE_GAMEPLAY_TAG(Fire, "Ability.Block.Fire");
	}

	namespace Ability
	{
		namespace CoolDown
		{
			UE_DEFINE_GAMEPLAY_TAG(Banana, "Ability.CoolDown.Banana");
		}
	}
	
	namespace Data
	{
		UE_DEFINE_GAMEPLAY_TAG(Damage, "Data.Damage");
		UE_DEFINE_GAMEPLAY_TAG(Duration, "Data.Duration");
	}
	
	namespace Event
	{
		UE_DEFINE_GAMEPLAY_TAG(Bomb_Explode, "Event.Bomb.Explode");
		UE_DEFINE_GAMEPLAY_TAG(Gimmick_Collect, "Event.Gimmick.Collect");
		UE_DEFINE_GAMEPLAY_TAG(Weapon_ReloadReFill, "Event.Weapon.ReloadReFill");
		
		namespace Round
		{
			UE_DEFINE_GAMEPLAY_TAG(Economy_Depression, "Event.Round.Economy.Depression");
			UE_DEFINE_GAMEPLAY_TAG(Economy_Inflation, "Event.Round.Economy.Inflation");
		}

		namespace Melee
		{
			UE_DEFINE_GAMEPLAY_TAG(Hit, "Event.Melee.Hit");
		}
	}
	
	namespace MiniGame
	{
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Bomb, "MiniGame.Bomb");
	}
	
	namespace Input
	{
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Crouch, "Input.Action.Crouch");
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Equip, "Input.Action.Equip");
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Fire, "Input.Action.Fire");
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Jump, "Input.Action.Jump");
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Reload, "Input.Action.Reload");
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Sprint, "Input.Action.Sprint");
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Drop, "Input.Action.Drop");
	}
	
	namespace State
	{
		PTW_API UE_DEFINE_GAMEPLAY_TAG(HitReaction_HeadShot, "State.HitReaction.HeadShot");
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Posture_Crouching, "State.Posture.Crouching");
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Status_Dead, "State.Dead");
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Status_Bomb, "State.BombAttach");
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Invincible, "State.Invincible");
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Stasis, "State.Stasis");
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Slowing, "State.DebuffSlow");
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Stealth, "State.Stealth");
		
		namespace Passive
		{
			PTW_API UE_DEFINE_GAMEPLAY_TAG(ReflectShield, "State.Passive.ReflectShield");
		}
		
		namespace Movement
		{
			PTW_API UE_DEFINE_GAMEPLAY_TAG(InAir, "State.Movement.InAir");
			PTW_API UE_DEFINE_GAMEPLAY_TAG(Sprinting, "State.Movement.Sprinting");
		}
		
	}
	
	namespace Item
	{
		namespace Passive
		{
			PTW_API UE_DEFINE_GAMEPLAY_TAG(ReflectShield, "Item.Passive.ReflectShield");
		}

		namespace Chaos
		{
			PTW_API UE_DEFINE_GAMEPLAY_TAG(Test, "Item.Chaos.Test");

		}
	}
	namespace Weapon
	{
		namespace Anim
		{
			PTW_API UE_DEFINE_GAMEPLAY_TAG(Fire, "Weapon.Anim.Fire");
			PTW_API UE_DEFINE_GAMEPLAY_TAG(Reload, "Weapon.Anim.Reload");
		}
		namespace Gun
		{
			namespace Pistol
			{
				PTW_API UE_DEFINE_GAMEPLAY_TAG(Pistol, "Weapon.Gun.Pistol");
				PTW_API UE_DEFINE_GAMEPLAY_TAG(BombPistol, "Weapon.Gun.Pistol.BombPistol");
			}

			namespace Rifle
			{
				PTW_API UE_DEFINE_GAMEPLAY_TAG(Rifle, "Weapon.Gun.Rifle");
				PTW_API UE_DEFINE_GAMEPLAY_TAG(WaterGun, "Weapon.Gun.Rifle.WaterGun");
			}
			PTW_API UE_DEFINE_GAMEPLAY_TAG(Rocket, "Weapon.Gun.Rocket");
		}
		namespace State
		{
			PTW_API UE_DEFINE_GAMEPLAY_TAG(Equip, "Weapon.State.Equip");
			PTW_API UE_DEFINE_GAMEPLAY_TAG(Reload, "Weapon.State.Reload");
			PTW_API UE_DEFINE_GAMEPLAY_TAG(UnEquip, "Weapon.State.UnEquip");
		}
		
		namespace EquipType
		{
			PTW_API UE_DEFINE_GAMEPLAY_TAG(Basic, "Weapon.EquipType.Basic");
			PTW_API UE_DEFINE_GAMEPLAY_TAG(Special, "Weapon.EquipType.Special");
		}
	}
	
	namespace GameplayCue
	{
		namespace Weapon
		{
			PTW_API UE_DEFINE_GAMEPLAY_TAG(Explosion, "GameplayCue.Weapon.Explosion");
			PTW_API UE_DEFINE_GAMEPLAY_TAG(Fire, "GameplayCue.Weapon.Fire");
			PTW_API UE_DEFINE_GAMEPLAY_TAG(HitImpact, "GameplayCue.Weapon.HitImpact");
			PTW_API UE_DEFINE_GAMEPLAY_TAG(Water, "GameplayCue.Weapon.Water");
		}
		
		namespace Item
		{
			PTW_API UE_DEFINE_GAMEPLAY_TAG(Invisibility, "GameplayCue.Item.Invisibility");
			PTW_API UE_DEFINE_GAMEPLAY_TAG(Invincible, "GameplayCue.Item.Invincible");
			PTW_API UE_DEFINE_GAMEPLAY_TAG(Stealth, "GameplayCue.Item.Stealth");
		}
		
		namespace Hit
		{
			PTW_API UE_DEFINE_GAMEPLAY_TAG(Wall, "GameplayCue.Hit.Wall");
		}
	}

	namespace ChaosEvent
	{
		namespace Buff
		{
			PTW_API UE_DEFINE_GAMEPLAY_TAG(MoveSpeed, "ChaosEvent.Buff.MoveSpeed");
		}
		
		namespace Debuff
		{
			PTW_API UE_DEFINE_GAMEPLAY_TAG(MoveSpeed, "ChaosEvent.Debuff.MoveSpeed");
		}
		
	}
	
}
