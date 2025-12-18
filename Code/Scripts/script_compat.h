/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	Script Commands Compatibility Header
**	Provides wrapper class with default arguments for function pointer calls
**	that originally relied on MSVC6 non-standard default arguments in function pointers.
**
**	This header redefines Commands to use a wrapper class.
**	Include AFTER scriptcommands.h but BEFORE any code that uses Commands->
*/

#ifndef SCRIPT_COMPAT_H
#define SCRIPT_COMPAT_H

#include "scriptcommands.h"

// The actual Commands pointer (original name preserved)
extern ScriptCommands* _RealCommands;

// Wrapper class that provides methods with default arguments
class ScriptCommandsWrapper {
public:
	// Pass-through all members from the underlying struct
	ScriptCommands* operator->() { return _RealCommands; }

	// Methods with default arguments - these shadow the struct members
	void Send_Custom_Event(GameObject* from, GameObject* to, int type, int param = 0, float delay = 0.0f) {
		_RealCommands->Send_Custom_Event(from, to, type, param, delay);
	}

	void Modify_Action(GameObject* obj, int action_id, const ActionParamsStruct& params, bool modify_move = true, bool modify_attack = true) {
		_RealCommands->Modify_Action(obj, action_id, params, modify_move, modify_attack);
	}

	void Set_Animation(GameObject* obj, const char* anim_name, bool looping = true, const char* sub_obj_name = NULL, float start_frame = 0.0f, float end_frame = -1.0f, bool is_blended = true) {
		_RealCommands->Set_Animation(obj, anim_name, looping, sub_obj_name, start_frame, end_frame, is_blended);
	}

	void Grant_Key(GameObject* obj, int key, bool grant = true) {
		_RealCommands->Grant_Key(obj, key, grant);
	}

	void Control_Enable(GameObject* obj, bool enable = true) {
		_RealCommands->Control_Enable(obj, enable);
	}

	void Set_Obj_Radar_Blip_Shape(GameObject* obj, int shape, int blip_color = 5) {
		_RealCommands->Set_Obj_Radar_Blip_Shape(obj, shape, blip_color);
	}

	void Set_Objective_Radar_Blip(int obj_id, int blip_type, int blip_obj_type = 0, int blip_color = 5, char* loc_str_desc = NULL, int sound_id = -1) {
		_RealCommands->Set_Objective_Radar_Blip(obj_id, blip_type, blip_obj_type, blip_color, loc_str_desc, sound_id);
	}

	void Create_2D_Wave_Sound_Dialog(const char* filename, const char* sound_obj_name = NULL) {
		_RealCommands->Create_2D_Wave_Sound_Dialog(filename, sound_obj_name);
	}

	void Enable_HUD_Pokable_Indicator(GameObject* obj, bool enable = true) {
		_RealCommands->Enable_HUD_Pokable_Indicator(obj, enable);
	}
};

// Global wrapper instance
extern ScriptCommandsWrapper CommandsWrapper;

// Macro to redirect Commands-> calls to use wrapper methods where available
// Note: This only works for the specific wrapped methods
// Other methods fall through via operator->

#endif // SCRIPT_COMPAT_H
