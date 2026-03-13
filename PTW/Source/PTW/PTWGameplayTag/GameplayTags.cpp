#include "GameplayTags.h"

namespace GameplayTags
{
	namespace AbilityBlockTag
	{
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Fire, "Ability.Block.Fire");
	}

	namespace Ability
	{
		namespace CoolDown
		{
			PTW_API UE_DEFINE_GAMEPLAY_TAG(Banana, "Ability.CoolDown.Banana");
		}
		
		namespace Action
		{
			PTW_API UE_DEFINE_GAMEPLAY_TAG(Emote, "Ability.Action.Emote");
		}
	}
	
	namespace Data
	{
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Damage, "Data.Damage");
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Duration, "Data.Duration");
	}
	
	namespace Event
	{
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Bomb_Explode, "Event.Bomb.Explode");
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Gimmick_Collect, "Event.Gimmick.Collect");
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Weapon_ReloadReFill, "Event.Weapon.ReloadReFill");
		
		namespace Round
		{
			PTW_API UE_DEFINE_GAMEPLAY_TAG(Economy_Depression, "Event.Round.Economy.Depression");
			PTW_API UE_DEFINE_GAMEPLAY_TAG(Economy_Inflation, "Event.Round.Economy.Inflation");
		}
		namespace Chaos
		{
			PTW_API UE_DEFINE_GAMEPLAY_TAG(DwarfPotion, "Event.Chaos.DwarfPotion");
		}
		namespace Melee
		{
			PTW_API UE_DEFINE_GAMEPLAY_TAG(Hit, "Event.Melee.Hit");
		}
	}
	
	namespace MiniGame
	{
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Bomb, "MiniGame.Bomb");
	}
	
	namespace Role
	{
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Citizen, "Role.Citizen");
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Cop, "Role.Cop");
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Robber, "Role.Robber");
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
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Emote, "Input.Action.Emote");
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Interact, "Input.Action.Interact");
	}
	
	namespace State
	{
		PTW_API UE_DEFINE_GAMEPLAY_TAG(HitReaction_HeadShot, "State.HitReaction.HeadShot");
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Posture_Crouching, "State.Posture.Crouching");
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Status_Dead, "State.Dead");
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Status_Blind, "State.Blind");
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Status_Bomb, "State.BombAttach");
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Invincible, "State.Invincible");
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Stasis, "State.Stasis");
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Slowing, "State.DebuffSlow");
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Stealth, "State.Stealth");
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Charge, "State.Charge");
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Emoting, "State.Emoting");
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Idle, "State.Idle");
		PTW_API UE_DEFINE_GAMEPLAY_TAG(Bomb, "State.Bomb");
		
		namespace Passive
		{
			PTW_API UE_DEFINE_GAMEPLAY_TAG(ReflectShield, "State.Passive.ReflectShield");
		}
		
		namespace Movement
		{
			PTW_API UE_DEFINE_GAMEPLAY_TAG(InAir, "State.Movement.InAir");
			PTW_API UE_DEFINE_GAMEPLAY_TAG(Sprinting, "State.Movement.Sprinting");
		}
		
		namespace Ghost
		{
			PTW_API UE_DEFINE_GAMEPLAY_TAG(Invisible, "State.Ghost.Invisible");
			PTW_API UE_DEFINE_GAMEPLAY_TAG(Revealed, "State.Ghost.Revealed");
		}
		
		namespace Posture
		{
			UE_DEFINE_GAMEPLAY_TAG(Crouching, "State.Posture.Crouching");
		}
	}
	
	namespace Item
	{
		namespace Passive
		{
			PTW_API UE_DEFINE_GAMEPLAY_TAG(ReflectShield, "Item.Passive.ReflectShield");
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
				PTW_API UE_DEFINE_GAMEPLAY_TAG(AbyssPistol, "Weapon.Gun.Pistol.AbyssPistol");
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
		
		namespace Melee
		{
			PTW_API UE_DEFINE_GAMEPLAY_TAG(Knife, "Weapon.Melee.Knife");
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
			PTW_API UE_DEFINE_GAMEPLAY_TAG(Empty, "GameplayCue.Weapon.Empty");
		}
		
		namespace Item
		{
			PTW_API UE_DEFINE_GAMEPLAY_TAG(Invisibility, "GameplayCue.Item.Invisibility");
			PTW_API UE_DEFINE_GAMEPLAY_TAG(Invincible, "GameplayCue.Item.Invincible");
			PTW_API UE_DEFINE_GAMEPLAY_TAG(Stealth, "GameplayCue.Item.Stealth");
		}
		
		namespace State
		{
			PTW_API UE_DEFINE_GAMEPLAY_TAG(Blind, "GameplayCue.State.Blind");
		}
		
		namespace Hit
		{
			PTW_API UE_DEFINE_GAMEPLAY_TAG(Wall, "GameplayCue.Hit.Wall");
		}
		
		namespace MiniGame
		{
			PTW_API UE_DEFINE_GAMEPLAY_TAG(Delivery, "GameplayCue.MiniGame.Delivery");
		}
	}

	namespace GameState
	{
		namespace MiniGame
		{
			PTW_API UE_DEFINE_GAMEPLAY_TAG(Bomb, "GameState.MiniGame.Bomb");
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
			PTW_API UE_DEFINE_GAMEPLAY_TAG(MoveSpeed, "ChaosEvent.DeBuff.MoveSpeed");
		}
	}
	
}
