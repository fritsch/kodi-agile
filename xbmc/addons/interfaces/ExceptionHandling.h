#pragma once
/*
 *      Copyright (C) 2016 Team KODI
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

#include "utils/log.h"
#include "commons/Exception.h"

#include <exception>

namespace ADDON
{

  class CAddonDll;

  void LogException(CAddonDll* addon, const std::exception &e, const char *name);

};

#define HANDLE_ADDON_EXCEPTION(addon) \
catch (std::exception &e) \
{ \
  ADDON::LogException(addon, e, __FUNCTION__); \
} \
catch (...) \
{ \
  CLog::Log(LOGERROR, "EXCEPTION: Unknown exception thrown from the call \"%s\"", __FUNCTION__); \
}

#define HANDLE_ADDON_EXCEPTION_WITH_RETURN(addon, ret) \
catch (std::exception &e) \
{ \
  ADDON::LogException(addon, e, __FUNCTION__); \
  return ret; \
} \
catch (...) \
{ \
  CLog::Log(LOGERROR, "EXCEPTION: Unknown exception thrown from the call \"%s\"", __FUNCTION__); \
  return ret; \
}