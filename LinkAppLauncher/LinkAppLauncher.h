
// LinkAppLauncher.h: PROJECT_NAME 애플리케이션에 대한 주 헤더 파일입니다.
//
#pragma once

#ifndef __AFXWIN_H__
#error "PCH에 대해 이 파일을 포함하기 전에 'pch.h'를 포함합니다."
#endif

#include "resource.h"
#include "logger.h"
#include <filesystem>

class CLinkAppLauncherApp : public CWinApp
{
public:
    CLinkAppLauncherApp();

public:
    virtual BOOL InitInstance();
    BOOL LaunchProgram(const CString& programName);

private:
    std::unique_ptr<Logger> logger_;
    CString ExtractParameter(const CString& cmdLine, const CString& param);
    BOOL EnsureDirectoryExists(const CString& path);

    DECLARE_MESSAGE_MAP()
};

extern CLinkAppLauncherApp theApp;