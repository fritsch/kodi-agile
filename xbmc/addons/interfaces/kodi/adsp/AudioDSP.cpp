/*
 *      Copyright (C) 2010-2015 Team Kodi
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

#include <vector>
#include "Application.h"
#include "AudioDSP.h"
#include "addons/interfaces/ExceptionHandling.h"
#include "cores/AudioEngine/Engines/ActiveAE/AudioDSPAddons/ActiveAEDSP.h"
#include "commons/Exception.h"
#include "settings/AdvancedSettings.h"
#include "settings/Settings.h"
#include "utils/log.h"
#include "utils/StringUtils.h"

using namespace ADDON;
using namespace ActiveAE;

#define DEFAULT_INFO_STRING_VALUE "unknown"

CActiveAEDSPAddon::CActiveAEDSPAddon(AddonProps props) :
    CAddonDll(std::move(props)),
    m_apiVersion("0.0.0"),
    m_addonInstance(nullptr)
{
  ResetProperties();
}

CActiveAEDSPAddon::~CActiveAEDSPAddon(void)
{
  Destroy();
}

void CActiveAEDSPAddon::OnDisabled()
{
  CServiceBroker::GetADSP().UpdateAddons();
}

void CActiveAEDSPAddon::OnEnabled()
{
  CServiceBroker::GetADSP().UpdateAddons();
}

AddonPtr CActiveAEDSPAddon::GetRunningInstance() const
{
  if (CServiceBroker::GetADSP().IsActivated())
  {
    AddonPtr adspAddon;
    if (CServiceBroker::GetADSP().GetAudioDSPAddon(ID(), adspAddon))
      return adspAddon;
  }
  return CAddon::GetRunningInstance();
}

void CActiveAEDSPAddon::OnPreInstall()
{
  CServiceBroker::GetADSP().UpdateAddons();
}

void CActiveAEDSPAddon::OnPostInstall(bool restart, bool update)
{
  CServiceBroker::GetADSP().UpdateAddons();
}

void CActiveAEDSPAddon::OnPreUnInstall()
{
  // stop the ADSP manager, so running ADSP add-ons are stopped and closed
  CServiceBroker::GetADSP().Deactivate();
}

void CActiveAEDSPAddon::OnPostUnInstall()
{
  CServiceBroker::GetADSP().UpdateAddons();
}

void CActiveAEDSPAddon::ResetProperties(int iClientId /* = AE_DSP_INVALID_ADDON_ID */)
{
  /* initialise members */
  m_strUserPath           = CSpecialProtocol::TranslatePath(Profile());
  m_strAddonPath          = CSpecialProtocol::TranslatePath(Path());
  m_menuhooks.clear();
  m_bReadyToUse           = false;
  m_isInUse               = false;
  m_iClientId             = iClientId;
  m_strAudioDSPVersion    = DEFAULT_INFO_STRING_VALUE;
  m_strFriendlyName       = DEFAULT_INFO_STRING_VALUE;
  m_strAudioDSPName       = DEFAULT_INFO_STRING_VALUE;
  memset(&m_addonCapabilities, 0, sizeof(m_addonCapabilities));
  m_apiVersion = AddonVersion("0.0.0");

  memset(&m_struct, 0, sizeof(m_struct));

  m_struct.props.strUserPath = m_strUserPath.c_str();
  m_struct.props.strAddonPath = m_strAddonPath.c_str();

  m_struct.toKodi.kodiInstance = this;
  m_struct.toKodi.AddMenuHook = ADSPAddMenuHook;
  m_struct.toKodi.RemoveMenuHook = ADSPRemoveMenuHook;
  m_struct.toKodi.RegisterMode = ADSPRegisterMode;
  m_struct.toKodi.UnregisterMode = ADSPUnregisterMode;
}

ADDON_STATUS CActiveAEDSPAddon::Create(int iClientId)
{
  ADDON_STATUS status(ADDON_STATUS_UNKNOWN);
  if (iClientId <= AE_DSP_INVALID_ADDON_ID)
    return status;

  /* ensure that a previous instance is destroyed */
  Destroy();

  /* reset all properties to defaults */
  ResetProperties(iClientId);

  /* initialise the add-on */
  bool bReadyToUse(false);
  CLog::Log(LOGDEBUG, "ActiveAE DSP - %s - creating audio dsp add-on instance '%s'", __FUNCTION__, Name().c_str());
  try
  {
    if ((status = CAddonDll::Create()) != ADDON_STATUS_OK)
      return status;

    if ((status = CAddonDll::CreateInstance(ADDON_INSTANCE_ADSP, ID().c_str(), &m_struct, &m_addonInstance)) != ADDON_STATUS_OK)
      return status;

    bReadyToUse = GetAddonProperties();
  }
  catch (std::exception& ex) { ExceptionStdHandle(ex, __FUNCTION__); }
  catch (int ex)             { ExceptionErrHandle(ex, __FUNCTION__); }
  catch (...)                { ExceptionUnkHandle(__FUNCTION__); }

  m_bReadyToUse = bReadyToUse;

  return status;
}

bool CActiveAEDSPAddon::DllLoaded(void) const
{
  return CAddonDll::DllLoaded();
}

void CActiveAEDSPAddon::Destroy(void)
{
  /* reset 'ready to use' to false */
  if (!m_bReadyToUse)
    return;
  m_bReadyToUse = false;

  CLog::Log(LOGDEBUG, "ActiveAE DSP - %s - destroying audio dsp add-on '%s'", __FUNCTION__, GetFriendlyName().c_str());

  /* destroy the add-on */
  try
  {
    CAddonDll::DestroyInstance(ADDON_INSTANCE_ADSP, m_addonInstance);
    CAddonDll::Destroy();
    m_addonInstance = nullptr;
  }
  catch (std::exception& ex) { ExceptionStdHandle(ex, __FUNCTION__); }
  catch (int ex)             { ExceptionErrHandle(ex, __FUNCTION__); }
  catch (...)                { ExceptionUnkHandle(__FUNCTION__); }

  /* reset all properties to defaults */
  ResetProperties();
}

void CActiveAEDSPAddon::ReCreate(void)
{
  int iClientID(m_iClientId);
  Destroy();

  /* recreate the instance */
  Create(iClientID);
}

bool CActiveAEDSPAddon::ReadyToUse(void) const
{
  return m_bReadyToUse;
}

int CActiveAEDSPAddon::GetID(void) const
{
  return m_iClientId;
}

bool CActiveAEDSPAddon::IsInUse() const
{
  return m_isInUse;
};

bool CActiveAEDSPAddon::GetAddonProperties(void)
{
  std::string strDSPName, strFriendlyName, strAudioDSPVersion;
  AE_DSP_ADDON_CAPABILITIES addonCapabilities;

  /* get the capabilities */
  try
  {
    memset(&addonCapabilities, 0, sizeof(addonCapabilities));
    m_struct.toAddon.GetCapabilities(m_addonInstance, &addonCapabilities);
  }
  catch (std::exception& ex) { ExceptionStdHandle(ex, "GetCapabilities()"); return false; }
  catch (int ex)             { ExceptionErrHandle(ex, "GetCapabilities()"); return false; }
  catch (...)                { ExceptionUnkHandle("GetCapabilities()"); return false; }

  /* get the name of the dsp addon */
  try
  {
    strDSPName = m_struct.toAddon.GetDSPName(m_addonInstance);
  }
  catch (std::exception& ex) { ExceptionStdHandle(ex, "GetDSPName()"); return false; }
  catch (int ex)             { ExceptionErrHandle(ex, "GetDSPName()"); return false; }
  catch (...)                { ExceptionUnkHandle("GetDSPName()"); return false; }


  /* display name = backend name string */
  strFriendlyName = StringUtils::Format("%s", strDSPName.c_str());

  /* backend version number */
  try
  {
    strAudioDSPVersion = m_struct.toAddon.GetDSPVersion(m_addonInstance);
  }
  catch (std::exception& ex) { ExceptionStdHandle(ex, "GetDSPVersion()"); return false; }
  catch (int ex)             { ExceptionErrHandle(ex, "GetDSPVersion()"); return false; }
  catch (...)                { ExceptionUnkHandle("GetDSPVersion()"); return false; }

  /* update the members */
  m_strAudioDSPName     = strDSPName;
  m_strFriendlyName     = strFriendlyName;
  m_strAudioDSPVersion  = strAudioDSPVersion;
  m_addonCapabilities   = addonCapabilities;

  return true;
}

AE_DSP_ADDON_CAPABILITIES CActiveAEDSPAddon::GetCapabilities(void) const
{
  AE_DSP_ADDON_CAPABILITIES addonCapabilities(m_addonCapabilities);
  return addonCapabilities;
}

const std::string &CActiveAEDSPAddon::GetAudioDSPName(void) const
{
  return m_strAudioDSPName;
}

const std::string &CActiveAEDSPAddon::GetAudioDSPVersion(void) const
{
  return m_strAudioDSPVersion;
}

const std::string &CActiveAEDSPAddon::GetFriendlyName(void) const
{
  return m_strFriendlyName;
}

bool CActiveAEDSPAddon::HaveMenuHooks(AE_DSP_MENUHOOK_CAT cat) const
{
  if (m_bReadyToUse && !m_menuhooks.empty())
  {
    for (unsigned int i = 0; i < m_menuhooks.size(); ++i)
    {
      if (m_menuhooks[i].category == cat || m_menuhooks[i].category == AE_DSP_MENUHOOK_ALL)
        return true;
    }
  }
  return false;
}

AE_DSP_MENUHOOKS *CActiveAEDSPAddon::GetMenuHooks(void)
{
  return &m_menuhooks;
}

void CActiveAEDSPAddon::CallMenuHook(const AE_DSP_MENUHOOK &hook, AE_DSP_MENUHOOK_DATA &hookData)
{
  if (!m_bReadyToUse || hookData.category == AE_DSP_MENUHOOK_UNKNOWN)
    return;

  try
  {
    m_struct.toAddon.MenuHook(m_addonInstance, hook, hookData);
  }
  catch (std::exception& ex) { ExceptionStdHandle(ex, "MenuHook()"); }
  catch (int ex)             { ExceptionErrHandle(ex, "MenuHook()"); }
  catch (...)                { ExceptionUnkHandle("MenuHook()"); }
}

AE_DSP_ERROR CActiveAEDSPAddon::StreamCreate(const AE_DSP_SETTINGS *addonSettings, const AE_DSP_STREAM_PROPERTIES* pProperties, ADDON_HANDLE handle)
{
  AE_DSP_ERROR retVal(AE_DSP_ERROR_UNKNOWN);

  try
  {
    retVal = m_struct.toAddon.StreamCreate(m_addonInstance, addonSettings, pProperties, handle);
    if (retVal == AE_DSP_ERROR_NO_ERROR)
      m_isInUse = true;
    LogError(retVal, __FUNCTION__);
  }
  catch (std::exception& ex) { ExceptionStdHandle(ex, __FUNCTION__); }
  catch (int ex)             { ExceptionErrHandle(ex, __FUNCTION__); }
  catch (...)                { ExceptionUnkHandle(__FUNCTION__); }

  return retVal;
}

void CActiveAEDSPAddon::StreamDestroy(const ADDON_HANDLE handle)
{
  try
  {
    m_struct.toAddon.StreamDestroy(m_addonInstance, handle);
  }
  catch (std::exception& ex) { ExceptionStdHandle(ex, __FUNCTION__); }
  catch (int ex)             { ExceptionErrHandle(ex, __FUNCTION__); }
  catch (...)                { ExceptionUnkHandle(__FUNCTION__); }

  m_isInUse = false;
}

bool CActiveAEDSPAddon::StreamIsModeSupported(const ADDON_HANDLE handle, AE_DSP_MODE_TYPE type, unsigned int addon_mode_id, int unique_db_mode_id)
{
  try
  {
    AE_DSP_ERROR retVal = m_struct.toAddon.StreamIsModeSupported(m_addonInstance, handle, type, addon_mode_id, unique_db_mode_id);
    if (retVal == AE_DSP_ERROR_NO_ERROR)
      return true;
    else if (retVal != AE_DSP_ERROR_IGNORE_ME)
      LogError(retVal, __FUNCTION__);
  }
  catch (std::exception& ex) { ExceptionStdHandle(ex, __FUNCTION__); }
  catch (int ex)             { ExceptionErrHandle(ex, __FUNCTION__); }
  catch (...)                { ExceptionUnkHandle(__FUNCTION__); }

  return false;
}

AE_DSP_ERROR CActiveAEDSPAddon::StreamInitialize(const ADDON_HANDLE handle, const AE_DSP_SETTINGS *addonSettings)
{
  AE_DSP_ERROR retVal(AE_DSP_ERROR_UNKNOWN);

  try
  {
    retVal = m_struct.toAddon.StreamInitialize(m_addonInstance, handle, addonSettings);
    LogError(retVal, __FUNCTION__);
  }
  catch (std::exception& ex) { ExceptionStdHandle(ex, __FUNCTION__); }
  catch (int ex)             { ExceptionErrHandle(ex, __FUNCTION__); }
  catch (...)                { ExceptionUnkHandle(__FUNCTION__); }

  return retVal;
}

bool CActiveAEDSPAddon::InputProcess(const ADDON_HANDLE handle, const float **array_in, unsigned int samples)
{
  try
  {
    return m_struct.toAddon.InputProcess(m_addonInstance, handle, array_in, samples);
  }
  catch (std::exception& ex) { ExceptionStdHandle(ex, __FUNCTION__); }
  catch (int ex)             { ExceptionErrHandle(ex, __FUNCTION__); }
  catch (...)                { ExceptionUnkHandle(__FUNCTION__); }

  return 0;
}

unsigned int CActiveAEDSPAddon::InputResampleProcessNeededSamplesize(const ADDON_HANDLE handle)
{
  try
  {
    return m_struct.toAddon.InputResampleProcessNeededSamplesize(m_addonInstance, handle);
  }
  catch (std::exception& ex) { ExceptionStdHandle(ex, __FUNCTION__); }
  catch (int ex)             { ExceptionErrHandle(ex, __FUNCTION__); }
  catch (...)                { ExceptionUnkHandle(__FUNCTION__); }

  return 0;
}

unsigned int CActiveAEDSPAddon::InputResampleProcess(const ADDON_HANDLE handle, float **array_in, float **array_out, unsigned int samples)
{
  try
  {
    return m_struct.toAddon.InputResampleProcess(m_addonInstance, handle, array_in, array_out, samples);
  }
  catch (std::exception& ex) { ExceptionStdHandle(ex, __FUNCTION__); }
  catch (int ex)             { ExceptionErrHandle(ex, __FUNCTION__); }
  catch (...)                { ExceptionUnkHandle(__FUNCTION__); }

  return 0;
}

int CActiveAEDSPAddon::InputResampleSampleRate(const ADDON_HANDLE handle)
{
  try
  {
    return m_struct.toAddon.InputResampleSampleRate(m_addonInstance, handle);
  }
  catch (std::exception& ex) { ExceptionStdHandle(ex, __FUNCTION__); }
  catch (int ex)             { ExceptionErrHandle(ex, __FUNCTION__); }
  catch (...)                { ExceptionUnkHandle(__FUNCTION__); }

  return -1;
}

float CActiveAEDSPAddon::InputResampleGetDelay(const ADDON_HANDLE handle)
{
  try
  {
    return m_struct.toAddon.InputResampleGetDelay(m_addonInstance, handle);
  }
  catch (std::exception& ex) { ExceptionStdHandle(ex, __FUNCTION__); }
  catch (int ex)             { ExceptionErrHandle(ex, __FUNCTION__); }
  catch (...)                { ExceptionUnkHandle(__FUNCTION__); }

  return 0.0f;
}

unsigned int CActiveAEDSPAddon::PreProcessNeededSamplesize(const ADDON_HANDLE handle, unsigned int mode_id)
{
  try
  {
    return m_struct.toAddon.PreProcessNeededSamplesize(m_addonInstance, handle, mode_id);
  }
  catch (std::exception& ex) { ExceptionStdHandle(ex, __FUNCTION__); }
  catch (int ex)             { ExceptionErrHandle(ex, __FUNCTION__); }
  catch (...)                { ExceptionUnkHandle(__FUNCTION__); }

  return 0;
}

float CActiveAEDSPAddon::PreProcessGetDelay(const ADDON_HANDLE handle, unsigned int mode_id)
{
  try
  {
    return m_struct.toAddon.PreProcessGetDelay(m_addonInstance, handle, mode_id);
  }
  catch (std::exception& ex) { ExceptionStdHandle(ex, __FUNCTION__); }
  catch (int ex)             { ExceptionErrHandle(ex, __FUNCTION__); }
  catch (...)                { ExceptionUnkHandle(__FUNCTION__); }

  return 0.0f;
}

unsigned int CActiveAEDSPAddon::PreProcess(const ADDON_HANDLE handle, unsigned int mode_id, float **array_in, float **array_out, unsigned int samples)
{
  try
  {
    return m_struct.toAddon.PostProcess(m_addonInstance, handle, mode_id, array_in, array_out, samples);
  }
  catch (std::exception& ex) { ExceptionStdHandle(ex, __FUNCTION__); }
  catch (int ex)             { ExceptionErrHandle(ex, __FUNCTION__); }
  catch (...)                { ExceptionUnkHandle(__FUNCTION__); }

  return 0;
}

AE_DSP_ERROR CActiveAEDSPAddon::MasterProcessSetMode(const ADDON_HANDLE handle, AE_DSP_STREAMTYPE type, unsigned int mode_id, int unique_db_mode_id)
{
  AE_DSP_ERROR retVal(AE_DSP_ERROR_UNKNOWN);

  try
  {
    retVal = m_struct.toAddon.MasterProcessSetMode(m_addonInstance, handle, type, mode_id, unique_db_mode_id);
    LogError(retVal, __FUNCTION__);
  }
  catch (std::exception& ex) { ExceptionStdHandle(ex, __FUNCTION__); }
  catch (int ex)             { ExceptionErrHandle(ex, __FUNCTION__); }
  catch (...)                { ExceptionUnkHandle(__FUNCTION__); }

  return retVal;
}

unsigned int CActiveAEDSPAddon::MasterProcessNeededSamplesize(const ADDON_HANDLE handle)
{
  try
  {
    return m_struct.toAddon.MasterProcessNeededSamplesize(m_addonInstance, handle);
  }
  catch (std::exception& ex) { ExceptionStdHandle(ex, __FUNCTION__); }
  catch (int ex)             { ExceptionErrHandle(ex, __FUNCTION__); }
  catch (...)                { ExceptionUnkHandle(__FUNCTION__); }

  return 0;
}

float CActiveAEDSPAddon::MasterProcessGetDelay(const ADDON_HANDLE handle)
{
  try
  {
    return m_struct.toAddon.MasterProcessGetDelay(m_addonInstance, handle);
  }
  catch (std::exception& ex) { ExceptionStdHandle(ex, __FUNCTION__); }
  catch (int ex)             { ExceptionErrHandle(ex, __FUNCTION__); }
  catch (...)                { ExceptionUnkHandle(__FUNCTION__); }

  return 0.0f;
}

int CActiveAEDSPAddon::MasterProcessGetOutChannels(const ADDON_HANDLE handle, unsigned long &out_channel_present_flags)
{
  try
  {
    return m_struct.toAddon.MasterProcessGetOutChannels(m_addonInstance, handle, out_channel_present_flags);
  }
  catch (std::exception& ex) { ExceptionStdHandle(ex, __FUNCTION__); }
  catch (int ex)             { ExceptionErrHandle(ex, __FUNCTION__); }
  catch (...)                { ExceptionUnkHandle(__FUNCTION__); }

  return -1;
}

unsigned int CActiveAEDSPAddon::MasterProcess(const ADDON_HANDLE handle, float **array_in, float **array_out, unsigned int samples)
{
  try
  {
    return m_struct.toAddon.MasterProcess(m_addonInstance, handle, array_in, array_out, samples);
  }
  catch (std::exception& ex) { ExceptionStdHandle(ex, __FUNCTION__); }
  catch (int ex)             { ExceptionErrHandle(ex, __FUNCTION__); }
  catch (...)                { ExceptionUnkHandle(__FUNCTION__); }

  return 0;
}

std::string CActiveAEDSPAddon::MasterProcessGetStreamInfoString(const ADDON_HANDLE handle)
{
  std::string strReturn;

  if (!m_bReadyToUse)
    return strReturn;

  try
  {
    strReturn = m_struct.toAddon.MasterProcessGetStreamInfoString(m_addonInstance, handle);
  }
  catch (std::exception& ex) { ExceptionStdHandle(ex, __FUNCTION__); }
  catch (int ex)             { ExceptionErrHandle(ex, __FUNCTION__); }
  catch (...)                { ExceptionUnkHandle(__FUNCTION__); }

  return strReturn;
}

unsigned int CActiveAEDSPAddon::PostProcessNeededSamplesize(const ADDON_HANDLE handle, unsigned int mode_id)
{
  try
  {
    return m_struct.toAddon.PostProcessNeededSamplesize(m_addonInstance, handle, mode_id);
  }
  catch (std::exception& ex) { ExceptionStdHandle(ex, __FUNCTION__); }
  catch (int ex)             { ExceptionErrHandle(ex, __FUNCTION__); }
  catch (...)                { ExceptionUnkHandle(__FUNCTION__); }

  return 0;
}

float CActiveAEDSPAddon::PostProcessGetDelay(const ADDON_HANDLE handle, unsigned int mode_id)
{
  try
  {
    return m_struct.toAddon.PostProcessGetDelay(m_addonInstance, handle, mode_id);
  }
  catch (std::exception& ex) { ExceptionStdHandle(ex, __FUNCTION__); }
  catch (int ex)             { ExceptionErrHandle(ex, __FUNCTION__); }
  catch (...)                { ExceptionUnkHandle(__FUNCTION__); }

  return 0.0f;
}

unsigned int CActiveAEDSPAddon::PostProcess(const ADDON_HANDLE handle, unsigned int mode_id, float **array_in, float **array_out, unsigned int samples)
{
  try
  {
    return m_struct.toAddon.PostProcess(m_addonInstance, handle, mode_id, array_in, array_out, samples);
  }
  catch (std::exception& ex) { ExceptionStdHandle(ex, __FUNCTION__); }
  catch (int ex)             { ExceptionErrHandle(ex, __FUNCTION__); }
  catch (...)                { ExceptionUnkHandle(__FUNCTION__); }

  return 0;
}

unsigned int CActiveAEDSPAddon::OutputResampleProcessNeededSamplesize(const ADDON_HANDLE handle)
{
  try
  {
    return m_struct.toAddon.OutputResampleProcessNeededSamplesize(m_addonInstance, handle);
  }
  catch (std::exception& ex) { ExceptionStdHandle(ex, __FUNCTION__); }
  catch (int ex)             { ExceptionErrHandle(ex, __FUNCTION__); }
  catch (...)                { ExceptionUnkHandle(__FUNCTION__); }

  return 0;
}

unsigned int CActiveAEDSPAddon::OutputResampleProcess(const ADDON_HANDLE handle, float **array_in, float **array_out, unsigned int samples)
{
  try
  {
    return m_struct.toAddon.OutputResampleProcess(m_addonInstance, handle, array_in, array_out, samples);
  }
  catch (std::exception& ex) { ExceptionStdHandle(ex, __FUNCTION__); }
  catch (int ex)             { ExceptionErrHandle(ex, __FUNCTION__); }
  catch (...)                { ExceptionUnkHandle(__FUNCTION__); }

  return 0;
}

int CActiveAEDSPAddon::OutputResampleSampleRate(const ADDON_HANDLE handle)
{
  try
  {
    return m_struct.toAddon.OutputResampleSampleRate(m_addonInstance, handle);
  }
  catch (std::exception& ex) { ExceptionStdHandle(ex, __FUNCTION__); }
  catch (int ex)             { ExceptionErrHandle(ex, __FUNCTION__); }
  catch (...)                { ExceptionUnkHandle(__FUNCTION__); }

  return -1;
}

float CActiveAEDSPAddon::OutputResampleGetDelay(const ADDON_HANDLE handle)
{
  try
  {
    return m_struct.toAddon.OutputResampleGetDelay(m_addonInstance, handle);
  }
  catch (std::exception& ex) { ExceptionStdHandle(ex, __FUNCTION__); }
  catch (int ex)             { ExceptionErrHandle(ex, __FUNCTION__); }
  catch (...)                { ExceptionUnkHandle(__FUNCTION__); }

  return 0.0f;
}

bool CActiveAEDSPAddon::SupportsInputInfoProcess(void) const
{
  return m_addonCapabilities.bSupportsInputProcess;
}

bool CActiveAEDSPAddon::SupportsInputResample(void) const
{
  return m_addonCapabilities.bSupportsInputResample;
}

bool CActiveAEDSPAddon::SupportsPreProcess(void) const
{
  return m_addonCapabilities.bSupportsPreProcess;
}

bool CActiveAEDSPAddon::SupportsMasterProcess(void) const
{
  return m_addonCapabilities.bSupportsMasterProcess;
}

bool CActiveAEDSPAddon::SupportsPostProcess(void) const
{
  return m_addonCapabilities.bSupportsPostProcess;
}

bool CActiveAEDSPAddon::SupportsOutputResample(void) const
{
  return m_addonCapabilities.bSupportsOutputResample;
}

const char *CActiveAEDSPAddon::ToString(const AE_DSP_ERROR error)
{
  switch (error)
  {
  case AE_DSP_ERROR_NO_ERROR:
    return "no error";
  case AE_DSP_ERROR_NOT_IMPLEMENTED:
    return "not implemented";
  case AE_DSP_ERROR_REJECTED:
    return "rejected by the backend";
  case AE_DSP_ERROR_INVALID_PARAMETERS:
    return "invalid parameters for this method";
  case AE_DSP_ERROR_INVALID_SAMPLERATE:
    return "invalid samplerate for this method";
  case AE_DSP_ERROR_INVALID_IN_CHANNELS:
    return "invalid input channel layout for this method";
  case AE_DSP_ERROR_INVALID_OUT_CHANNELS:
    return "invalid output channel layout for this method";
  case AE_DSP_ERROR_FAILED:
    return "the command failed";
  case AE_DSP_ERROR_UNKNOWN:
  default:
    return "unknown error";
  }
}

bool CActiveAEDSPAddon::LogError(const AE_DSP_ERROR error, const char *strMethod) const
{
  if (error != AE_DSP_ERROR_NO_ERROR && error != AE_DSP_ERROR_IGNORE_ME)
  {
    CLog::Log(LOGERROR, "ActiveAE DSP - %s - addon '%s' returned an error: %s",
        strMethod, GetFriendlyName().c_str(), ToString(error));
    return false;
  }
  return true;
}

void CActiveAEDSPAddon::ADSPAddMenuHook(void *kodiInstance, AE_DSP_MENUHOOK *hook)
{
  CActiveAEDSPAddon* addon = static_cast<CActiveAEDSPAddon*>(kodiInstance);
  if (!hook || !addon)
  {
    CLog::Log(LOGERROR, "Audio DSP - %s - invalid handler data", __FUNCTION__);
    return;
  }

  AE_DSP_MENUHOOKS *hooks = addon->GetMenuHooks();
  if (hooks)
  {
    AE_DSP_MENUHOOK hookInt;
    hookInt.iHookId            = hook->iHookId;
    hookInt.iLocalizedStringId = hook->iLocalizedStringId;
    hookInt.category           = hook->category;
    hookInt.iRelevantModeId    = hook->iRelevantModeId;
    hookInt.bNeedPlayback      = hook->bNeedPlayback;

    /* add this new hook */
    hooks->push_back(hookInt);
  }
}

void CActiveAEDSPAddon::ADSPRemoveMenuHook(void *kodiInstance, AE_DSP_MENUHOOK *hook)
{
  CActiveAEDSPAddon* addon = static_cast<CActiveAEDSPAddon*>(kodiInstance);
  if (!hook || !addon)
  {
    CLog::Log(LOGERROR, "Audio DSP - %s - invalid handler data", __FUNCTION__);
    return;
  }

  AE_DSP_MENUHOOKS *hooks = addon->GetMenuHooks();
  if (hooks)
  {
    for (unsigned int i = 0; i < hooks->size(); i++)
    {
      if (hooks->at(i).iHookId == hook->iHookId)
      {
        /* remove this hook */
        hooks->erase(hooks->begin()+i);
        break;
      }
    }
  }
}

void CActiveAEDSPAddon::ADSPRegisterMode(void* kodiInstance, AE_DSP_MODES::AE_DSP_MODE* mode)
{
  CActiveAEDSPAddon* addon = static_cast<CActiveAEDSPAddon*>(kodiInstance);
  if (!mode || !addon)
  {
    CLog::Log(LOGERROR, "Audio DSP - %s - invalid mode data", __FUNCTION__);
    return;
  }

  CActiveAEDSPMode transferMode(*mode, addon->GetID());
  int idMode = transferMode.AddUpdate();
  mode->iUniqueDBModeId = idMode;

  if (idMode > AE_DSP_INVALID_ADDON_ID)
  {
    CLog::Log(LOGDEBUG, "Audio DSP - %s - successfull registered mode %s of %s adsp-addon", __FUNCTION__, mode->strModeName, addon->Name().c_str());
  }
  else
  {
    CLog::Log(LOGERROR, "Audio DSP - %s - failed to register mode %s of %s adsp-addon", __FUNCTION__, mode->strModeName, addon->Name().c_str());
  }
}

void CActiveAEDSPAddon::ADSPUnregisterMode(void* kodiInstance, AE_DSP_MODES::AE_DSP_MODE* mode)
{
  CActiveAEDSPAddon* addon = static_cast<CActiveAEDSPAddon*>(kodiInstance);
  if (!mode || !addon)
  {
    CLog::Log(LOGERROR, "Audio DSP - %s - invalid mode data", __FUNCTION__);
    return;
  }

  CActiveAEDSPMode transferMode(*mode, addon->GetID());
  transferMode.Delete();
}

void CActiveAEDSPAddon::ExceptionStdHandle(std::exception& ex, const char* function)
{
  Exception::LogStdException(this, ex, function);
  Destroy();
  CAddonMgr::GetInstance().DisableAddon(this->ID());
  Exception::ShowExceptionErrorDialog(this);
}

void CActiveAEDSPAddon::ExceptionErrHandle(int ex, const char* function)
{
  Exception::LogErrException(this, ex, function);
  Destroy();
  CAddonMgr::GetInstance().DisableAddon(this->ID());
  Exception::ShowExceptionErrorDialog(this);
}

void CActiveAEDSPAddon::ExceptionUnkHandle(const char* function)
{
  Exception::LogUnkException(this, function);
  Destroy();
  CAddonMgr::GetInstance().DisableAddon(this->ID());
  Exception::ShowExceptionErrorDialog(this);
}