#pragma once
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

struct sFuncTable_Addon;

namespace ADDON
{
extern "C"
{

  struct Interface_GUIControlSpin
  {
  public:
    static void Init(sFuncTable_Addon* funcTable);

    static void SetVisible(void* kodiInstance, void* handle, bool visible);
    static void SetEnabled(void* kodiInstance, void* handle, bool enabled);

    static void SetText(void* kodiInstance, void* handle, const char *text);
    static void Reset(void* kodiInstance, void* handle);
    static void SetType(void* kodiInstance, void* handle, int type);

    static void AddStringLabel(void* kodiInstance, void* handle, const char* label, const char* value);
    static void AddIntLabel(void* kodiInstance, void* handle, const char* label, int value);

    static void SetStringValue(void* kodiInstance, void* handle, const char* value);
    static void GetStringValue(void* kodiInstance, void* handle, char &value, unsigned int &maxStringSize);

    static void SetIntRange(void* kodiInstance, void* handle, int start, int end);
    static void SetIntValue(void* kodiInstance, void* handle, int value);
    static int GetIntValue(void* kodiInstance, void* handle);

    static void SetFloatRange(void* kodiInstance, void* handle, float start, float end);
    static void SetFloatValue(void* kodiInstance, void* handle, float value);
    static float GetFloatValue(void* kodiInstance, void* handle);
    static void SetFloatInterval(void* kodiInstance, void* handle, float interval);
  };

} /* extern "C" */
} /* namespace ADDON */