/*
 *      Copyright (C) 2015-2016 Team KODI
 *      http://kodi.tv
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with KODI; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "DialogYesNo.h"
#include "addons/kodi-addon-dev-kit/include/kodi/gui/DialogYesNo.h"

#include "addons/AddonDll.h"
#include "addons/interfaces/ExceptionHandling.h"
#include "dialogs/GUIDialogYesNo.h"
#include "messaging/helpers/DialogHelper.h"

using namespace KODI::MESSAGING;
using KODI::MESSAGING::HELPERS::DialogResponse;

namespace ADDON
{
extern "C"
{

void Interface_GUIDialogYesNo::Init(sFuncTable_Addon* funcTable)
{
  funcTable->toKodi.kodi_gui->dialogYesNo.ShowAndGetInputSingleText = ShowAndGetInputSingleText;
  funcTable->toKodi.kodi_gui->dialogYesNo.ShowAndGetInputLineText = ShowAndGetInputLineText;
  funcTable->toKodi.kodi_gui->dialogYesNo.ShowAndGetInputLineButtonText = ShowAndGetInputLineButtonText;
}

bool Interface_GUIDialogYesNo::ShowAndGetInputSingleText(void* kodiInstance,
                                                         const char *heading,
                                                         const char *text,
                                                         bool &bCanceled,
                                                         const char *noLabel,
                                                         const char *yesLabel)
{
  CAddonDll* addon = static_cast<CAddonDll*>(kodiInstance);
  if (!addon)
  {
    CLog::Log(LOGERROR, "Interface_GUIDialogYesNo::%s - invalid data", __FUNCTION__);
    return false;
  }

  try
  {
    DialogResponse result = HELPERS::ShowYesNoDialogText(heading, text, noLabel, yesLabel);
    bCanceled = (result == DialogResponse::CANCELLED);
    return (result == DialogResponse::YES);
  }
  HANDLE_ADDON_EXCEPTION(addon);

  return false;
}

bool Interface_GUIDialogYesNo::ShowAndGetInputLineText(void* kodiInstance,
                                                       const char *heading,
                                                       const char *line0,
                                                       const char *line1,
                                                       const char *line2,
                                                       const char *noLabel,
                                                       const char *yesLabel)
{
  CAddonDll* addon = static_cast<CAddonDll*>(kodiInstance);
  if (!addon)
  {
    CLog::Log(LOGERROR, "Interface_GUIDialogYesNo::%s - invalid data", __FUNCTION__);
    return false;
  }

  try
  {
    return HELPERS::ShowYesNoDialogLines(heading, line0, line1, line2, noLabel, yesLabel) ==
      DialogResponse::YES;
  }
  HANDLE_ADDON_EXCEPTION(addon);

  return false;
}

bool Interface_GUIDialogYesNo::ShowAndGetInputLineButtonText(void* kodiInstance,
                                                             const char *heading,
                                                             const char *line0,
                                                             const char *line1,
                                                             const char *line2,
                                                             bool &bCanceled,
                                                             const char *noLabel,
                                                             const char *yesLabel)
{
  CAddonDll* addon = static_cast<CAddonDll*>(kodiInstance);
  if (!addon)
  {
    CLog::Log(LOGERROR, "Interface_GUIDialogYesNo::%s - invalid data", __FUNCTION__);
    return false;
  }

  try
  {
    DialogResponse result = HELPERS::ShowYesNoDialogLines(heading, line0, line1, line2, noLabel, yesLabel);
    bCanceled = (result == DialogResponse::CANCELLED);
    return (result == DialogResponse::YES);
  }
  HANDLE_ADDON_EXCEPTION(addon);

  return false;
}

} /* extern "C" */
} /* namespace ADDON */