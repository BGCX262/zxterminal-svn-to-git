// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once



#ifdef _WINDOWS
	#include "targetver.h"
	#define WIN32_LEAN_AND_MEAN             // �� Windows ͷ���ų�����ʹ�õ�����
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

