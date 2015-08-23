// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once



#ifdef _WINDOWS
	#include "targetver.h"
	#define WIN32_LEAN_AND_MEAN             // 从 Windows 头中排除极少使用的资料
	#include <windows.h>

//	#ifdef WIN64
//		#ifdef _DEBUG
//			#define __LOG__
//				#ifdef __LOG__ 
//					#pragma comment(lib, "../../lib/Debug-64bit/log4cppD.lib")
//				#endif
//
//		#else
//			//#define __LOG__
//			//#ifdef __LOG__ 
//			#pragma comment(lib, "../../lib/release-64bit/log4cpp.lib")
//			//#endif
//		#endif
//
//	#else	//WIN32
//
//		#ifdef _DEBUG
//			//#define __LOG__
//			#ifdef __LOG__ 
//				#pragma comment(lib, "../../lib/Debug/log4cppD.lib")
//			#endif
//		#else
//			//#define __LOG__
//			//#ifdef __LOG__ 
//				#pragma comment(lib, "../../lib/release/log4cpp.lib")
//			//#endif
//		#endif
//	#endif
//
//#else	//Linux

#endif

