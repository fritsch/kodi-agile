/*
 *      Copyright (C) 2012-2013 Team XBMC
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

#include "Codec.h"

#include "utils/StringUtils.h"

extern "C" {
#include "libavcodec/avcodec.h"
}

#include <utility>

namespace ADDON
{

class CCodecIds
{
public:
  virtual ~CCodecIds(void) {}

  static CCodecIds& GetInstance()
  {
    static CCodecIds _instance;
    return _instance;
  }

  xbmc_codec_t GetCodecByName(const char* strCodecName)
  {
    xbmc_codec_t retVal = XBMC_INVALID_CODEC;
    if (strlen(strCodecName) == 0)
      return retVal;

    std::string strUpperCodecName = strCodecName;
    StringUtils::ToUpper(strUpperCodecName);

    std::map<std::string, xbmc_codec_t>::const_iterator it = m_lookup.find(strUpperCodecName);
    if (it != m_lookup.end())
      retVal = it->second;

    return retVal;
  }

private:
  CCodecIds(void)
  {
    // get ids and names
    AVCodec* codec = NULL;
    xbmc_codec_t tmp;
    while ((codec = av_codec_next(codec)))
    {
      if (av_codec_is_decoder(codec))
      {
        tmp.codec_type = (xbmc_codec_type_t)codec->type;
        tmp.codec_id   = codec->id;

        std::string strUpperCodecName = codec->name;
        StringUtils::ToUpper(strUpperCodecName);

        m_lookup.insert(std::make_pair(strUpperCodecName, tmp));
      }
    }

    // teletext is not returned by av_codec_next. we got our own decoder
    tmp.codec_type = XBMC_CODEC_TYPE_SUBTITLE;
    tmp.codec_id   = AV_CODEC_ID_DVB_TELETEXT;
    m_lookup.insert(std::make_pair("TELETEXT", tmp));

    // rds is not returned by av_codec_next. we got our own decoder
    tmp.codec_type = XBMC_CODEC_TYPE_RDS;
    tmp.codec_id   = AV_CODEC_ID_NONE;
    m_lookup.insert(std::make_pair("RDS", tmp));
  }

  std::map<std::string, xbmc_codec_t> m_lookup;
};

void Interface_Codec::Init(sFuncTable_Addon *funcTable)
{
  funcTable->toKodi.kodi_codec = (sAddonToKodiFuncTable_kodi_codec*)malloc(sizeof(sAddonToKodiFuncTable_kodi_codec));
  funcTable->toKodi.kodi_codec->get_codec_by_name = get_codec_by_name;
}

void Interface_Codec::DeInit(sFuncTable_Addon* funcTable)
{
  if (funcTable->toKodi.kodi_codec)
  {
    free(funcTable->toKodi.kodi_codec);
    funcTable->toKodi.kodi_codec = nullptr;
  }
}

xbmc_codec_t Interface_Codec::get_codec_by_name(void* kodiInstance, const char* strCodecName)
{
  return CCodecIds::GetInstance().GetCodecByName(strCodecName);
}

} /* namespace ADDON */