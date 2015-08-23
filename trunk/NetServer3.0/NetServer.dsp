# Microsoft Developer Studio Project File - Name="NetServer" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=NetServer - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "NetServer.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "NetServer.mak" CFG="NetServer - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "NetServer - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "NetServer - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "NetServer - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 Ws2_32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "NetServer - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 Ws2_32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "NetServer - Win32 Release"
# Name "NetServer - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ado2.cpp
# End Source File
# Begin Source File

SOURCE=.\BaseEquipment.cpp
# End Source File
# Begin Source File

SOURCE=.\BaseNetAgreement.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientManager.cpp
# End Source File
# Begin Source File

SOURCE=.\DBWork.cpp
# End Source File
# Begin Source File

SOURCE=.\DoUserEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\EquimentList.cpp
# End Source File
# Begin Source File

SOURCE=.\EquipmentManager.cpp
# End Source File
# Begin Source File

SOURCE=.\GmemoryDataManager.cpp
# End Source File
# Begin Source File

SOURCE=.\GTQEquipment.cpp
# End Source File
# Begin Source File

SOURCE=.\IOCPBuff.cpp
# End Source File
# Begin Source File

SOURCE=.\Log.cpp
# End Source File
# Begin Source File

SOURCE=.\MemoryPool.cpp
# End Source File
# Begin Source File

SOURCE=.\MTGPSEquipment.cpp
# End Source File
# Begin Source File

SOURCE=.\MTJJAEquipment.cpp
# End Source File
# Begin Source File

SOURCE=.\Net.cpp
# End Source File
# Begin Source File

SOURCE=.\NetAgreement.cpp
# End Source File
# Begin Source File

SOURCE=.\NetEquipment.cpp
# End Source File
# Begin Source File

SOURCE=.\NetNode.cpp
# End Source File
# Begin Source File

SOURCE=.\NetReadThread.cpp
# End Source File
# Begin Source File

SOURCE=.\NetServer.cpp
# End Source File
# Begin Source File

SOURCE=.\NetServer.rc
# End Source File
# Begin Source File

SOURCE=.\NetServerDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\NetWorkThread.cpp
# End Source File
# Begin Source File

SOURCE=.\SocketService.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\SystemInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\Thread.cpp
# End Source File
# Begin Source File

SOURCE=.\ThreadMsgManager.cpp
# End Source File
# Begin Source File

SOURCE=.\TQEquipment.cpp
# End Source File
# Begin Source File

SOURCE=.\WorkNetData.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ado2.h
# End Source File
# Begin Source File

SOURCE=.\BaseEquipment.h
# End Source File
# Begin Source File

SOURCE=.\BaseNetAgreement.h
# End Source File
# Begin Source File

SOURCE=.\ClientManager.h
# End Source File
# Begin Source File

SOURCE=.\CommField.h
# End Source File
# Begin Source File

SOURCE=.\DBWork.h
# End Source File
# Begin Source File

SOURCE=.\DoUserEvent.h
# End Source File
# Begin Source File

SOURCE=.\EquimentList.h
# End Source File
# Begin Source File

SOURCE=.\EquipmentManager.h
# End Source File
# Begin Source File

SOURCE=.\GmemoryDataManager.h
# End Source File
# Begin Source File

SOURCE=.\GpsFiled.h
# End Source File
# Begin Source File

SOURCE=.\GTQEquipment.h
# End Source File
# Begin Source File

SOURCE=.\GuardFiled.h
# End Source File
# Begin Source File

SOURCE=.\HeadFile.h
# End Source File
# Begin Source File

SOURCE=.\IOCPBuff.h
# End Source File
# Begin Source File

SOURCE=.\Log.h
# End Source File
# Begin Source File

SOURCE=.\MemoryPool.h
# End Source File
# Begin Source File

SOURCE=.\MTGPSEquipment.h
# End Source File
# Begin Source File

SOURCE=.\MTJJAEquipment.h
# End Source File
# Begin Source File

SOURCE=.\Net.h
# End Source File
# Begin Source File

SOURCE=.\NetAgreement.h
# End Source File
# Begin Source File

SOURCE=.\NetEquipment.h
# End Source File
# Begin Source File

SOURCE=.\NetField.h
# End Source File
# Begin Source File

SOURCE=.\NetNode.h
# End Source File
# Begin Source File

SOURCE=.\NetReadThread.h
# End Source File
# Begin Source File

SOURCE=.\NetServer.h
# End Source File
# Begin Source File

SOURCE=.\NetServerDlg.h
# End Source File
# Begin Source File

SOURCE=.\NetWorkThread.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\ReturnNumber.h
# End Source File
# Begin Source File

SOURCE=.\SocketService.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\SystemInfo.h
# End Source File
# Begin Source File

SOURCE=.\Thread.h
# End Source File
# Begin Source File

SOURCE=.\ThreadMsgManager.h
# End Source File
# Begin Source File

SOURCE=.\TQEquipment.h
# End Source File
# Begin Source File

SOURCE=.\WorkNetData.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Net.ico
# End Source File
# Begin Source File

SOURCE=.\res\NetServer.ico
# End Source File
# Begin Source File

SOURCE=.\res\NetServer.rc2
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
