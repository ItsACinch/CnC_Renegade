/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/dlgconfigvideotab.cpp               $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/07/02 6:29p                                               $*
 *                                                                                             *
 *                    $Revision:: 10                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "dlgconfigvideotab.h"
#include "resource.h"
#include "ww3d.h"
#include "sliderctrl.h"
#include "dx8wrapper.h"
#include "cardinalspline.h"
#include "string_ids.h"
#include "comboboxctrl.h"
#include "rddesc.h"
#include "_globals.h"
#include <stdio.h>


////////////////////////////////////////////////////////////////
//	Static member initialization
////////////////////////////////////////////////////////////////
int DlgConfigVideoTabClass::GammaLevel		  = GAMMA_SLIDER_DEFAULT;
int DlgConfigVideoTabClass::BrightnessLevel = BRIGHTNESS_SLIDER_DEFAULT;
int DlgConfigVideoTabClass::ContrastLevel	  = CONTRAST_SLIDER_DEFAULT;
bool DlgConfigVideoTabClass::NeedsRestart   = false;

// Settings file path
static char SettingsFilePath[MAX_PATH] = {0};

////////////////////////////////////////////////////////////////
//
//	DlgConfigVideoTabClass
//
////////////////////////////////////////////////////////////////
DlgConfigVideoTabClass::DlgConfigVideoTabClass (void)	:
	ChildDialogClass (IDD_CONFIG_VIDEO),
	UpdateGamma (true),
	InitialWidth(0),
	InitialHeight(0),
	InitialDepth(0),
	InitialWindowed(false),
	SelectedWidth(0),
	SelectedHeight(0),
	SelectedDepth(0),
	SettingsChanged(false)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~DlgConfigVideoTabClass
//
////////////////////////////////////////////////////////////////
DlgConfigVideoTabClass::~DlgConfigVideoTabClass (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
DlgConfigVideoTabClass::On_Init_Dialog (void)
{
	//
	//	Get the current resolution
	//
	int width			= 0;
	int height			= 0;
	int bits			= 0;
	bool is_windowed	= false;
	WW3D::Get_Device_Resolution (width, height, bits, is_windowed);

	// Store initial settings
	InitialWidth = width;
	InitialHeight = height;
	InitialDepth = bits;
	InitialWindowed = is_windowed;
	SelectedWidth = width;
	SelectedHeight = height;
	SelectedDepth = bits;
	SettingsChanged = false;

	//
	//	Configure the display driver combo (just show current, keep disabled)
	//
	ComboBoxCtrlClass *driver_combo = (ComboBoxCtrlClass *)Get_Dlg_Item (IDC_DISPLAY_DRIVER);
	if (driver_combo != NULL) {
		int curr_device = WW3D::Get_Render_Device ();
		StringClass ascii_device_name = WW3D::Get_Render_Device_Name (curr_device);
		WideStringClass device_name;
		device_name.Convert_From (ascii_device_name);
		driver_combo->Add_String (device_name);
		driver_combo->Set_Curr_Sel (0);
	}
	Enable_Dlg_Item (IDC_DISPLAY_DRIVER, false);  // Keep driver selection disabled

	//
	//	Configure the resolution combo box
	//
	Configure_Resolution_Combobox();

	//
	//	Configure the bit depth combo box
	//
	Configure_BitDepth_Combobox();

	SliderCtrlClass *slider;

	// To avoid partial (and incorrect) gamma updates, disable it until all values have been correctly set.
	UpdateGamma = false;

	//
	//	Configure the gamma slider
	//
	slider = (SliderCtrlClass *)Get_Dlg_Item (IDC_GAMMA_SLIDER);
	slider->Set_Range (GAMMA_SLIDER_MIN, GAMMA_SLIDER_MAX);
	slider->Set_Pos (GammaLevel);

	//
	//	Configure the brightness slider
	//
	slider = (SliderCtrlClass *)Get_Dlg_Item (IDC_BRIGHTNESS_SLIDER);
	slider->Set_Range (BRIGHTNESS_SLIDER_MIN, BRIGHTNESS_SLIDER_MAX);
	slider->Set_Pos (BrightnessLevel);

	//
	//	Configure the contrast slider
	//
	slider = (SliderCtrlClass *)Get_Dlg_Item (IDC_CONTRAST_SLIDER);
	slider->Set_Range (CONTRAST_SLIDER_MIN, CONTRAST_SLIDER_MAX);

	// Now the gamma can take effect.
	UpdateGamma = true;
	slider->Set_Pos (ContrastLevel);

	ChildDialogClass::On_Init_Dialog ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Configure_Resolution_Combobox
//
////////////////////////////////////////////////////////////////
void
DlgConfigVideoTabClass::Configure_Resolution_Combobox (void)
{
	ComboBoxCtrlClass *combo_box = (ComboBoxCtrlClass *)Get_Dlg_Item (IDC_RESOLUTION);
	if (combo_box == NULL) {
		return;
	}

	// Get current resolution
	int curr_width = 0, curr_height = 0, curr_bits = 0;
	bool is_windowed = false;
	WW3D::Get_Device_Resolution (curr_width, curr_height, curr_bits, is_windowed);

	// Get available resolutions
	const RenderDeviceDescClass& desc = WW3D::Get_Render_Device_Desc();
	const DynamicVectorClass<ResolutionDescClass> & resos = desc.Enumerate_Resolutions();

	int curr_sel = 0;
	for (int i = 0; i < resos.Count(); ++i) {
		// Only show resolutions that match our current bit depth
		if (resos[i].BitDepth == curr_bits) {
			WideStringClass resolution;
			resolution.Format (L"%d x %d", resos[i].Width, resos[i].Height);
			combo_box->Add_String (resolution);

			// Check if this is the current resolution
			if (resos[i].Width == curr_width && resos[i].Height == curr_height) {
				curr_sel = combo_box->Get_Item_Count() - 1;
			}
		}
	}

	combo_box->Set_Curr_Sel (curr_sel);
}


////////////////////////////////////////////////////////////////
//
//	Configure_BitDepth_Combobox
//
////////////////////////////////////////////////////////////////
void
DlgConfigVideoTabClass::Configure_BitDepth_Combobox (void)
{
	ComboBoxCtrlClass *combo_box = (ComboBoxCtrlClass *)Get_Dlg_Item (IDC_BIT_DEPTH);
	if (combo_box == NULL) {
		return;
	}

	// Get current bit depth
	int curr_width = 0, curr_height = 0, curr_bits = 0;
	bool is_windowed = false;
	WW3D::Get_Device_Resolution (curr_width, curr_height, curr_bits, is_windowed);

	// Add common bit depths
	combo_box->Add_String (L"16");
	combo_box->Add_String (L"32");

	// Select current
	if (curr_bits == 16) {
		combo_box->Set_Curr_Sel (0);
	} else {
		combo_box->Set_Curr_Sel (1);
	}
}


////////////////////////////////////////////////////////////////
//
//	On_End_Dialog
//
////////////////////////////////////////////////////////////////
void
DlgConfigVideoTabClass::On_Destroy (void)
{
	GammaLevel		 = ((SliderCtrlClass *)Get_Dlg_Item (IDC_GAMMA_SLIDER))->Get_Pos();
	BrightnessLevel = ((SliderCtrlClass *)Get_Dlg_Item (IDC_BRIGHTNESS_SLIDER))->Get_Pos();
	ContrastLevel	 = ((SliderCtrlClass *)Get_Dlg_Item (IDC_CONTRAST_SLIDER))->Get_Pos();

	// Check if resolution settings changed
	if (SettingsChanged) {
		// Save pending settings to file
		Save_Pending_Settings(SelectedWidth, SelectedHeight, SelectedDepth, InitialWindowed ? 1 : 0);

		// Set flag so parent dialog can show restart message
		NeedsRestart = true;
	}
}


////////////////////////////////////////////////////////////////
//
//	On_SliderCtrl_Pos_Changed
//
////////////////////////////////////////////////////////////////
void
DlgConfigVideoTabClass::On_SliderCtrl_Pos_Changed
(
	SliderCtrlClass *	slider_ctrl,
	int					ctrl_id,
	int					new_pos
)
{
	const WCHAR *formatstring = L"%.2f";
		
	WideStringClass settingstring;

	if (UpdateGamma) {
	
		int g, b, c;	
		
	 	g = ((SliderCtrlClass *)Get_Dlg_Item (IDC_GAMMA_SLIDER))->Get_Pos();
		b = ((SliderCtrlClass *)Get_Dlg_Item (IDC_BRIGHTNESS_SLIDER))->Get_Pos();
		c = ((SliderCtrlClass *)Get_Dlg_Item (IDC_CONTRAST_SLIDER))->Get_Pos();
		Update_Gamma (g, b, c);
	}

	switch (ctrl_id) {

		case IDC_GAMMA_SLIDER:
			settingstring.Format (formatstring, Gamma_Scale (slider_ctrl->Get_Pos()));
			Set_Dlg_Item_Text (IDC_GAMMA_SETTING, settingstring);
			break;
	
		case IDC_BRIGHTNESS_SLIDER:
			settingstring.Format (formatstring, Gamma_Scale (slider_ctrl->Get_Pos()));
			Set_Dlg_Item_Text (IDC_BRIGHTNESS_SETTING, settingstring);
			break;
	
		case IDC_CONTRAST_SLIDER:
			settingstring.Format (formatstring, Gamma_Scale (slider_ctrl->Get_Pos()));
			Set_Dlg_Item_Text (IDC_CONTRAST_SETTING, settingstring);
			break;

		default:
			// Do nothing.
			break;
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_ComboBoxCtrl_Sel_Change
//
////////////////////////////////////////////////////////////////
void
DlgConfigVideoTabClass::On_ComboBoxCtrl_Sel_Change
(
	ComboBoxCtrlClass *	combo_ctrl,
	int					ctrl_id,
	int					old_sel,
	int					new_sel
)
{
	if (ctrl_id == IDC_RESOLUTION) {
		// Parse the selected resolution string to get width/height
		WideStringClass sel_text;
		combo_ctrl->Get_String(new_sel, sel_text);

		// Convert to ASCII and parse
		int w = 0, h = 0;
		if (swscanf(sel_text.Peek_Buffer(), L"%d x %d", &w, &h) == 2) {
			SelectedWidth = w;
			SelectedHeight = h;
			if (w != InitialWidth || h != InitialHeight) {
				SettingsChanged = true;
			}
		}
	}
	else if (ctrl_id == IDC_BIT_DEPTH) {
		// Parse the selected bit depth
		WideStringClass sel_text;
		combo_ctrl->Get_String(new_sel, sel_text);

		int d = 0;
		if (swscanf(sel_text.Peek_Buffer(), L"%d", &d) == 1) {
			SelectedDepth = d;
			if (d != InitialDepth) {
				SettingsChanged = true;
			}
		}
	}
}


////////////////////////////////////////////////////////////////
//
//	Get_Settings_Path - Returns path to settings file
//
////////////////////////////////////////////////////////////////
const char*
DlgConfigVideoTabClass::Get_Settings_Path (void)
{
	if (SettingsFilePath[0] == 0) {
		char temp_path[MAX_PATH];
		GetTempPathA(MAX_PATH, temp_path);
		sprintf(SettingsFilePath, "%sRenegade", temp_path);
		CreateDirectoryA(SettingsFilePath, NULL);
		strcat(SettingsFilePath, "\\graphics.ini");
	}
	return SettingsFilePath;
}


////////////////////////////////////////////////////////////////
//
//	Save_Pending_Settings - Save resolution settings to file
//
////////////////////////////////////////////////////////////////
bool
DlgConfigVideoTabClass::Save_Pending_Settings (int width, int height, int depth, int windowed)
{
	const char* path = Get_Settings_Path();

	FILE* f = fopen(path, "w");
	if (f == NULL) {
		return false;
	}

	fprintf(f, "[Graphics]\n");
	fprintf(f, "Width=%d\n", width);
	fprintf(f, "Height=%d\n", height);
	fprintf(f, "Depth=%d\n", depth);
	fprintf(f, "Windowed=%d\n", windowed);

	fclose(f);
	return true;
}


////////////////////////////////////////////////////////////////
//
//	Load_Pending_Settings - Load resolution settings from file
//
////////////////////////////////////////////////////////////////
bool
DlgConfigVideoTabClass::Load_Pending_Settings (int &width, int &height, int &depth, int &windowed)
{
	const char* path = Get_Settings_Path();

	FILE* f = fopen(path, "r");
	if (f == NULL) {
		return false;
	}

	char line[256];
	width = -1;
	height = -1;
	depth = -1;
	windowed = -1;

	while (fgets(line, sizeof(line), f) != NULL) {
		if (strncmp(line, "Width=", 6) == 0) {
			width = atoi(line + 6);
		} else if (strncmp(line, "Height=", 7) == 0) {
			height = atoi(line + 7);
		} else if (strncmp(line, "Depth=", 6) == 0) {
			depth = atoi(line + 6);
		} else if (strncmp(line, "Windowed=", 9) == 0) {
			windowed = atoi(line + 9);
		}
	}

	fclose(f);

	// Return true only if all values were read
	return (width > 0 && height > 0 && depth > 0 && windowed >= 0);
}


////////////////////////////////////////////////////////////////
//
//	Has_Pending_Settings - Check if settings file exists
//
////////////////////////////////////////////////////////////////
bool
DlgConfigVideoTabClass::Has_Pending_Settings (void)
{
	const char* path = Get_Settings_Path();
	FILE* f = fopen(path, "r");
	if (f != NULL) {
		fclose(f);
		return true;
	}
	return false;
}


////////////////////////////////////////////////////////////////
//
//	Clear_Pending_Settings - Delete the settings file
//
////////////////////////////////////////////////////////////////
void
DlgConfigVideoTabClass::Clear_Pending_Settings (void)
{
	const char* path = Get_Settings_Path();
	DeleteFileA(path);
}


