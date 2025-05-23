#include "../qcommon/qcommon.h"
#include "../bgame/bg_public.h"

scr_const_t scr_const;

/*
===============
GScr_LoadConsts
===============
*/
void GScr_LoadConsts()
{
	scr_const.emptystring = GScr_AllocString("");
	scr_const.allies = GScr_AllocString("allies");
	scr_const.axis = GScr_AllocString("axis");
	scr_const.current = GScr_AllocString("current");
	scr_const.damage = GScr_AllocString("damage");
	scr_const.death = GScr_AllocString("death");
	scr_const.dlight = GScr_AllocString("dlight");
	scr_const.done = GScr_AllocString("done");
	scr_const.empty = GScr_AllocString("empty");
	scr_const.entity = GScr_AllocString("entity");
	scr_const.failed = GScr_AllocString("failed");
	scr_const.fraction = GScr_AllocString("fraction");
	scr_const.goal = GScr_AllocString("goal");
	scr_const.grenade = GScr_AllocString("grenade");
	scr_const.info_notnull = GScr_AllocString("info_notnull");
	scr_const.invisible = GScr_AllocString("invisible");
	scr_const.key1 = GScr_AllocString("key1");
	scr_const.key2 = GScr_AllocString("key2");
	scr_const.killanimscript = GScr_AllocString("killanimscript");
	scr_const.left = GScr_AllocString("left");
	scr_const.movedone = GScr_AllocString("movedone");
	scr_const.noclass = GScr_AllocString("noclass");
	scr_const.normal = GScr_AllocString("normal");
	scr_const.pistol = GScr_AllocString("pistol");
	scr_const.plane_waypoint = GScr_AllocString("plane_waypoint");
	scr_const.player = GScr_AllocString("player");
	scr_const.position = GScr_AllocString("position");
	scr_const.primary = GScr_AllocString("primary");
	scr_const.primaryb = GScr_AllocString("primaryb");
	scr_const.prone = GScr_AllocString("prone");
	scr_const.right = GScr_AllocString("right");
	scr_const.rocket = GScr_AllocString("rocket");
	scr_const.rotatedone = GScr_AllocString("rotatedone");
	scr_const.script_brushmodel = GScr_AllocString("script_brushmodel");
	scr_const.script_model = GScr_AllocString("script_model");
	scr_const.script_origin = GScr_AllocString("script_origin");
	scr_const.spectator = GScr_AllocString("spectator");
	scr_const.stand = GScr_AllocString("stand");
	scr_const.surfacetype = GScr_AllocString("surfacetype");
	scr_const.target_script_trigger = GScr_AllocString("target_script_trigger");
	scr_const.tempEntity = GScr_AllocString("tempEntity");
	scr_const.touch = GScr_AllocString("touch");
	scr_const.trigger = GScr_AllocString("trigger");
	scr_const.trigger_use = GScr_AllocString("trigger_use");
	scr_const.trigger_use_touch = GScr_AllocString("trigger_use_touch");
	scr_const.trigger_damage = GScr_AllocString("trigger_damage");
	scr_const.trigger_lookat = GScr_AllocString("trigger_lookat");
	scr_const.truck_cam = GScr_AllocString("truck_cam");
	scr_const.worldspawn = GScr_AllocString("worldspawn");
	scr_const.binocular_enter = GScr_AllocString("binocular_enter");
	scr_const.binocular_exit = GScr_AllocString("binocular_exit");
	scr_const.binocular_fire = GScr_AllocString("binocular_fire");
	scr_const.binocular_release = GScr_AllocString("binocular_release");
	scr_const.binocular_drop = GScr_AllocString("binocular_drop");
	scr_const.begin = GScr_AllocString("begin");
	scr_const.intermission = GScr_AllocString("intermission");
	scr_const.menuresponse = GScr_AllocString("menuresponse");
	scr_const.playing = GScr_AllocString("playing");
	scr_const.none = GScr_AllocString("none");
	scr_const.dead = GScr_AllocString("dead");
	scr_const.auto_change = GScr_AllocString("auto_change");
	scr_const.manual_change = GScr_AllocString("manual_change");
	scr_const.freelook = GScr_AllocString("freelook");
	scr_const.call_vote = GScr_AllocString("call_vote");
	scr_const.vote = GScr_AllocString("vote");
	scr_const.snd_enveffectsprio_level = GScr_AllocString("snd_enveffectsprio_level");
	scr_const.snd_enveffectsprio_shellshock = GScr_AllocString("snd_enveffectsprio_shellshock");
	scr_const.snd_channelvolprio_holdbreath = GScr_AllocString("snd_channelvolprio_holdbreath");
	scr_const.snd_channelvolprio_pain = GScr_AllocString("snd_channelvolprio_pain");
	scr_const.snd_channelvolprio_shellshock = GScr_AllocString("snd_channelvolprio_shellshock");
	scr_const.tag_flash = GScr_AllocString("tag_flash");
	scr_const.tag_flash_11 = GScr_AllocString("tag_flash_11");
	scr_const.tag_flash_2 = GScr_AllocString("tag_flash_2");
	scr_const.tag_flash_22 = GScr_AllocString("tag_flash_22");
	scr_const.tag_brass = GScr_AllocString("tag_brass");
	scr_const.j_head = GScr_AllocString("j_head");
	scr_const.tag_weapon = GScr_AllocString("tag_weapon");
	scr_const.tag_player = GScr_AllocString("tag_player");
	scr_const.tag_camera = GScr_AllocString("tag_camera");
	scr_const.tag_aim = GScr_AllocString("tag_aim");
	scr_const.tag_aim_animated = GScr_AllocString("tag_aim_animated");
	scr_const.tag_origin = GScr_AllocString("tag_origin");
	scr_const.tag_butt = GScr_AllocString("tag_butt");
	scr_const.tag_weapon_right = GScr_AllocString("tag_weapon_right");
	scr_const.back_low = GScr_AllocString("back_low");
	scr_const.back_mid = GScr_AllocString("back_mid");
	scr_const.back_up = GScr_AllocString("back_up");
	scr_const.neck = GScr_AllocString("neck");
	scr_const.head = GScr_AllocString("head");
	scr_const.pelvis = GScr_AllocString("pelvis");
}
