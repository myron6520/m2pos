#include "m2pos_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>

// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <VersionHelpers.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <memory>
#include <sstream>
#include <SetupAPI.h>
#include <iostream>
#include <map>
#include <vector>

#include <Dbt.h>
// Link with Setupapi.lib
#pragma comment(lib, "Setupapi.lib")

namespace m2pos
{
static std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> g_methodChannel;
LRESULT CALLBACK USBDeviceNotificationCallback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DEVICECHANGE:
            if (wParam == DBT_DEVICEARRIVAL) {
                std::cout << "USB device inserted" << std::endl;
                if (g_methodChannel) {
                    g_methodChannel->InvokeMethod("usbPrinterConnected", nullptr);
                }
                // 处理USB设备插入事件
            } else if (wParam == DBT_DEVICEREMOVECOMPLETE) {
                std::cout << "USB device removed" << std::endl;
                // 处理USB设备拔出事件
                if (g_methodChannel) {
                    g_methodChannel->InvokeMethod("usbPrinterDisconnected", nullptr);
                }
            }
            break;
        default:
            break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
  // static
  void M2posPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarWindows *registrar)
  {
    g_methodChannel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "m2pos",
            &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<M2posPlugin>();

    g_methodChannel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result)
        {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(plugin));

    WNDCLASS wc = {};
    wc.lpfnWndProc = USBDeviceNotificationCallback;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"USBNotificationWindowClass";
    RegisterClass(&wc);

    HWND hwnd = CreateWindow(wc.lpszClassName, NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);

    // 注册USB设备通知
    DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;
    ZeroMemory(&NotificationFilter, sizeof(NotificationFilter));
    NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    NotificationFilter.dbcc_classguid = {0x28d78fad, 0x5a12, 0x11D1, 0xae, 0x5b, 0x00, 0x00, 0xf8, 0x03, 0xa8, 0xc2};
    RegisterDeviceNotification(hwnd, &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
  }

  M2posPlugin::M2posPlugin() {}

  M2posPlugin::~M2posPlugin() {}
  const GUID usbDeviceInterfaceGUID = {0x28d78fad, 0x5a12, 0x11D1, 0xae, 0x5b, 0x00, 0x00, 0xf8, 0x03, 0xa8, 0xc2};

  void M2posPlugin::HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result)
  {
    if (method_call.method_name().compare("writeData") == 0)
    {
      const auto &arguments = std::get<flutter::EncodableMap>(*method_call.arguments());
      int pid = std::get<int>(arguments.at(flutter::EncodableValue("pid")));
      int vid = std::get<int>(arguments.at(flutter::EncodableValue("vid")));
      std::cout << "Value of pid: " << pid << std::endl;
      std::cout << "Value of vid: " << vid << std::endl;

      HDEVINFO deviceInfoSet = SetupDiGetClassDevs(&usbDeviceInterfaceGUID, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
      if (deviceInfoSet == INVALID_HANDLE_VALUE)
      {
        std::cerr << "Error: SetupDiGetClassDevs failed." << std::endl;
      }
      SP_DEVINFO_DATA deviceInfoData;
      deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
      char DeviceName[MAX_PATH];
      bool didFound = false;
      for (DWORD i = 0; SetupDiEnumDeviceInfo(deviceInfoSet, i, &deviceInfoData); ++i)
      {
        DWORD dataSize;
        WCHAR buffer[1024];
        if (SetupDiGetDeviceRegistryProperty(deviceInfoSet, &deviceInfoData, SPDRP_HARDWAREID, NULL, (PBYTE)buffer, sizeof(buffer), &dataSize))
        {
          std::wstring hardwareId(buffer);
          size_t pidPosition = hardwareId.find(L"PID_");
          if (pidPosition != std::wstring::npos)
          {
            DWORD pidW = std::wcstoul(&hardwareId[pidPosition + 4], nullptr, 16);
            int p_id = static_cast<int>(pidW);
            if (p_id != pid)
            {
              continue;
            }
          }
          size_t vidPosition = hardwareId.find(L"VID_");
          if (vidPosition != std::wstring::npos)
          {
            DWORD vidW = std::wcstoul(&hardwareId[vidPosition + 4], nullptr, 16);
            int v_id = static_cast<int>(vidW);
            if (v_id != vid)
            {
              continue;
            }
          }
        }

        SP_INTERFACE_DEVICE_DATA Interface_Info;
        Interface_Info.cbSize = sizeof(Interface_Info);
        if (!SetupDiEnumInterfaceDevice(deviceInfoSet, NULL, (LPGUID)&usbDeviceInterfaceGUID, 0, &Interface_Info))
        {
          continue;
        }
        DWORD needed; // get the required length
        SetupDiGetInterfaceDeviceDetail(deviceInfoSet, &Interface_Info, NULL, 0, &needed, NULL);
        PSP_INTERFACE_DEVICE_DETAIL_DATA detail = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc(needed);
        detail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

        std::cout << "needed :" << needed << std::endl;
        if (!SetupDiGetInterfaceDeviceDetail(deviceInfoSet, &Interface_Info, detail, needed, NULL, NULL))
        {
          free((PVOID)detail);
          continue;
        }
        std::cout << "detail->DevicePath:" << detail->DevicePath << std::endl;

        char *ptr = (char *)(detail->DevicePath);
        memset(DeviceName, 0x00, MAX_PATH);
        for (int j = 0; j < MAX_PATH; j++)
        {
          DeviceName[j] = ptr[j];
          if ('}' == DeviceName[j])
          {
            break;
          }
        }
        free((PVOID)detail);
        didFound = true;
        break;
      }
      SetupDiDestroyDeviceInfoList(deviceInfoSet);
      if (didFound)
      {
        HANDLE hUsb = CreateFile((LPCWSTR)DeviceName,
                                 GENERIC_READ | GENERIC_WRITE,
                                 FILE_SHARE_WRITE | FILE_SHARE_READ,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_FLAG_OVERLAPPED,
                                 NULL);

        if (hUsb == INVALID_HANDLE_VALUE)
        {
          std::wcout << "Failed to open device: " << std::endl;
          std::cerr << "Failed to open device. Error code: " << GetLastError() << std::endl;
        }
        std::cout << "Open USB device successfully: " << std::endl;
        std::cout << "Device handle: " << hUsb << std::endl;
        const auto &uintList = std::get<std::vector<uint8_t>>(arguments.at(flutter::EncodableValue("data")));
        int nBytes;
        OVERLAPPED overlap;
        memset(&overlap, 0, sizeof(overlap));
        DWORD numread;
        const uint8_t *ptr = uintList.data();
        LPCVOID lpcData = reinterpret_cast<LPCVOID>(ptr);
        std::cout << "WriteFile  success  " << static_cast<DWORD>(uintList.size()) << std::endl;
        int success = WriteFile(hUsb, lpcData, static_cast<DWORD>(uintList.size()), (PULONG)&nBytes, &overlap);
        if (success)
        {
          std::cout << "WriteFile  success" << std::endl;
          std::cout << "WriteFile len:" << nBytes << std::endl;
        }
        else
        {

          std::cout << "WriteFile  fail" << GetLastError() << std::endl;
          if (GetLastError() == ERROR_IO_PENDING)
          {
            WaitForSingleObject(hUsb, 500);
            success = GetOverlappedResult(hUsb, &overlap, &numread, FALSE);
          }
          else
          {
          }
        }
        CloseHandle(hUsb);
      }
    }
    if (method_call.method_name().compare("readUsbPrinterList") == 0)
    {
      const auto res = ReadUsbPrinterList();
      flutter::EncodableList rawData;
      for(const auto &map :res){
        flutter::EncodableMap encodableMap;
        for(const auto &pair : map){
          encodableMap[flutter::EncodableValue(pair.first)] = flutter::EncodableValue(pair.second);
        }
        rawData.push_back(flutter::EncodableValue(encodableMap));
      }
      result->Success(flutter::EncodableValue(rawData));
    }
    if (method_call.method_name().compare("getPlatformVersion") == 0)
    {
      std::ostringstream version_stream;
      version_stream << "Windows ";
      if (IsWindows10OrGreater())
      {
        version_stream << "10+";
      }
      else if (IsWindows8OrGreater())
      {
        version_stream << "8";
      }
      else if (IsWindows7OrGreater())
      {
        version_stream << "7";
      }
      result->Success(flutter::EncodableValue(version_stream.str()));
    }
  }
  std::vector<std::map<std::string, std::string>> M2posPlugin::ReadUsbPrinterList()
  {

    std::vector<std::map<std::string, std::string>> dataList;
    // 获取设备信息集合
    HDEVINFO deviceInfoSet = SetupDiGetClassDevs(&usbDeviceInterfaceGUID, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (deviceInfoSet == INVALID_HANDLE_VALUE)
    {
      std::cerr << "Error: SetupDiGetClassDevs failed." << std::endl;
      return dataList;
    }

    // 枚举设备信息
    SP_DEVINFO_DATA deviceInfoData;
    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    for (DWORD i = 0; SetupDiEnumDeviceInfo(deviceInfoSet, i, &deviceInfoData); ++i)
    {

      std::map<std::string, std::string> info;
      // 获取设备路径
      WCHAR deviceInstanceId[1024];
      if (!SetupDiGetDeviceInstanceId(deviceInfoSet, &deviceInfoData, deviceInstanceId, 1024, NULL))
      {
        std::cerr << "Error: SetupDiGetDeviceInstanceId failed." << std::endl;
        continue;
      }
      // 打印设备路径
      std::wcout << "Device Instance ID: " << deviceInstanceId << std::endl;

      WCHAR deviceDesc[MAX_PATH];
      if (SetupDiGetDeviceRegistryProperty(deviceInfoSet, &deviceInfoData, SPDRP_DEVICEDESC, NULL, (PBYTE)deviceDesc, sizeof(deviceDesc), NULL))
      {
        std::wcout << L"This device is a printer: " << deviceDesc << std::endl;
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, deviceDesc, -1, nullptr, 0, nullptr, nullptr);
        std::string result(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, deviceDesc, -1, &result[0], size_needed, nullptr, nullptr);
        info["name"] = result;
      }

      DWORD dataSize;
      WCHAR buffer[1024];

      if (SetupDiGetDeviceRegistryProperty(deviceInfoSet, &deviceInfoData, SPDRP_HARDWAREID, NULL, (PBYTE)buffer, sizeof(buffer), &dataSize))
      {
        std::wstring hardwareId(buffer);
        std::wcout << "deviceDesc: " << deviceDesc << std::endl;
        size_t pidPosition = hardwareId.find(L"PID_");
        if (pidPosition != std::wstring::npos)
        {
          DWORD pid = std::wcstoul(&hardwareId[pidPosition + 4], nullptr, 16);
          std::wcout << L"Product ID (PID): 0x" << std::hex << pid << std::endl;
          std::string strValue = std::to_string(static_cast<unsigned int>(pid));
          info["pid"] = strValue;
          std::cout << "PID :" << strValue << std::endl;
        }
        size_t vidPosition = hardwareId.find(L"VID_");
        if (vidPosition != std::wstring::npos)
        {
          DWORD vid = std::wcstoul(&hardwareId[vidPosition + 4], nullptr, 16);
          std::wcout << L"Product ID (VID): 0x" << std::hex << vid << std::endl;
          std::string strValue = std::to_string(static_cast<unsigned int>(vid));
          info["vid"] = strValue;

          std::cout << "VID :" << strValue << std::endl;
        }
        dataList.push_back(info);
      }
    }

    for (const auto &map : dataList)
    {
      for (const auto &pair : map)
      {
        std::cout << pair.first << ":" << pair.second << ",";
      }
      std::cout << std::endl;
    }
    // 释放设备信息集合
    SetupDiDestroyDeviceInfoList(deviceInfoSet);

    return dataList;
  }

} // namespace m2pos
