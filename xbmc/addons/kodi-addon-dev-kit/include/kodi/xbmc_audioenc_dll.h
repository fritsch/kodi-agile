#pragma once

/*
 *      Copyright (C) 2005-2015 Team Kodi
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
 *  along with Kodi; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include <stdint.h>
#include "xbmc_addon_dll.h"
#include "xbmc_audioenc_types.h"

extern "C"
{
  //! \copydoc AudioEncoder::Create
  void* Create(void* addonInstance, sAddonToKodiFuncTable_AudioEncoder *callbacks);

  //! \copydoc AudioEncoder::Start
  bool Start(void* addonInstance, void* context, int iInChannels, int iInRate, int iInBits,
             const char* title, const char* artist,
             const char* albumartist, const char* album,
             const char* year, const char* track,
             const char* genre, const char* comment, int iTrackLength);

  //! \copydoc AudioEncoder::Encode
  int Encode(void* addonInstance, void* context, int nNumBytesRead, uint8_t* pbtStream);

  //! \copydoc AudioEncoder::Finish
  bool Finish(void* addonInstance, void* context);

  //! \copydoc AudioEncoder::Free
  void Free(void* addonInstance, void* context);

  // function to export the above structure to XBMC
  void SetAudioEncoderFuncTable(struct sKodiToAddonFuncTable_AudioEncoder* pScr)
  {
    pScr->Create = Create;
    pScr->Start  = Start;
    pScr->Encode = Encode;
    pScr->Finish = Finish;
    pScr->Free   = Free;
  };
};

