
// LinkAppLauncher.cpp: 애플리케이션에 대한 클래스 동작을 정의합니다.
//
#include "pch.h"
#include "framework.h"
#include "LinkAppLauncher.h"
#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CLinkAppLauncherApp, CWinApp)
    ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

CLinkAppLauncherApp::CLinkAppLauncherApp()
{
    m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
}

CLinkAppLauncherApp theApp;

BOOL CLinkAppLauncherApp::EnsureDirectoryExists(const CString& path)
{
    try {
        // Get directory path from the full file path
        CString directoryPath = path;
        int lastSlash = directoryPath.ReverseFind('\\');
        if (lastSlash != -1) {
            directoryPath = directoryPath.Left(lastSlash);
        }

        // Skip if directory already exists
        if (directoryPath.IsEmpty() || CreateDirectory(directoryPath, NULL)) {
            return TRUE;
        }

        // If creation failed, check if it's because directory already exists
        DWORD error = GetLastError();
        if (error == ERROR_ALREADY_EXISTS) {
            return TRUE;
        }

        // For nested directories, create them recursively
        CString parentPath;
        int position = 0;
        BOOL success = TRUE;

        while ((position = directoryPath.Find('\\', position + 1)) != -1) {
            parentPath = directoryPath.Left(position);
            if (!parentPath.IsEmpty()) {
                CreateDirectory(parentPath, NULL);
                // Ignore ERROR_ALREADY_EXISTS
                DWORD error = GetLastError();
                if (error != ERROR_ALREADY_EXISTS && error != ERROR_SUCCESS) {
                    success = FALSE;
                    break;
                }
            }
        }

        // Create the final directory
        if (success) {
            CreateDirectory(directoryPath, NULL);
            error = GetLastError();
            return (error == ERROR_SUCCESS || error == ERROR_ALREADY_EXISTS);
        }

        return FALSE;
    }
    catch (const std::exception&) {
        return FALSE;
    }
}

CString CLinkAppLauncherApp::ExtractParameter(const CString& cmdLine, const CString& param)
{
    CString paramLower = param;
    CString cmdLineLower = cmdLine;
    paramLower.MakeLower();
    cmdLineLower.MakeLower();

    int pos = cmdLineLower.Find(paramLower);
    if (pos != -1)
    {
        CString value = cmdLine.Mid(pos + param.GetLength());
        value.TrimLeft();

        int spacePos = value.Find(_T(' '));
        if (spacePos != -1)
        {
            value = value.Left(spacePos);
        }
        return value;
    }
    return _T("");
}

BOOL CLinkAppLauncherApp::LaunchProgram(const CString& programName)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    CString fullPath;
    fullPath.Format(_T("C:\\Windows\\System32\\%s.exe"), programName);

    if (logger_)
    {
        CString logMessage;
        logMessage.Format(_T("Attempting to launch program: %s"), fullPath);
        logger_->log(logMessage);
    }

    BOOL success = CreateProcess(
        fullPath,
        NULL,
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &si,
        &pi
    );

    if (success)
    {
        if (logger_)
        {
            logger_->log(_T("Program launched successfully"));
        }

        WaitForInputIdle(pi.hProcess, 1000);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return TRUE;
    }
    else
    {
        if (logger_)
        {
            DWORD error = GetLastError();
            CString errorMessage;
            errorMessage.Format(_T("Failed to launch program. Error code: %d"), error);
            logger_->log(errorMessage);
        }
        return FALSE;
    }
}

BOOL CLinkAppLauncherApp::InitInstance()
{
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(InitCtrls);
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&InitCtrls);

    CWinApp::InitInstance();

    CString cmdLine = GetCommandLine();

    // Extract log path
    CString logPath = ExtractParameter(cmdLine, _T("-lpath"));
    if (!logPath.IsEmpty())
    {
        if (EnsureDirectoryExists(logPath))
        {
            logger_ = std::make_unique<Logger>(logPath);
            logger_->log(_T("Application started"));
        }
        else
        {
            TRACE(_T("Failed to create directory for log file: %s\n"), (LPCTSTR)logPath);
        }
    }

    // Extract program type
    CString programName = ExtractParameter(cmdLine, _T("-type"));
    if (!programName.IsEmpty())
    {
        if (logger_)
        {
            CString logMessage;
            logMessage.Format(_T("Received program name: %s"), programName);
            logger_->log(logMessage);
        }

        LaunchProgram(programName);
    }
    else if (logger_)
    {
        logger_->log(_T("No program name specified"));
    }

    if (logger_)
    {
        logger_->log(_T("Application ending"));
    }

    return FALSE;
}