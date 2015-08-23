// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the MAPXREPLACE_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// MAPXREPLACE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef MAPXREPLACE_EXPORTS
#define MAPXREPLACE_API __declspec(dllexport)
#else
#define MAPXREPLACE_API __declspec(dllimport)
#endif

// This class is exported from the MapXReplace.dll
#include <string>
#include <map>
#include <algorithm>
#include <Windows.h>

class MAPXREPLACE_API CMapXReplace {
public:
	class MapXPoint
	{
	public:
		double longitude;//����
		double latitude;//γ��
		MapXPoint() : longitude(0.0f), latitude(0.0f){}
		MapXPoint(double lon, double lat) : longitude(lon), latitude(lat){}
		MapXPoint(const MapXPoint& _Right)
		{
			this->latitude = _Right.latitude;
			this->longitude = _Right.longitude;
		}
		const MapXPoint& operator=(const MapXPoint& _Right)
		{
			this->latitude = _Right.latitude;
			this->longitude = _Right.longitude;
			return *this;
		}
	};

	class MapXLayer
	{
	public:
		MapXPoint* points;
		int pointNum;
		MapXLayer(MapXPoint* points, int pointNum) : pointNum(pointNum)
		{
			this->points = new MapXPoint[pointNum];
			for(int i = 0; i < pointNum; i++)
				*(this->points + i) = points[i];
		}
		~MapXLayer()
		{
			if(this->points != NULL)
			{
				delete[] this->points;
				this->points = NULL;
			}
			this->pointNum = 0;
		}
	};//ͼ��

	typedef enum
	{
		MapXOp_Init,             //��ʼֵ��������
		MapXOp_Para_Err,         //��������
		MapXOp_Add_Fail,         //���ͼ��ʧ��
		MapXOp_Add_Exist,        //���ͼ��ʧ�ܣ��Ѿ�����
		MapXOp_Add_Succ,         //���ͼ��ɹ�
		MapXOp_PIL_InvalidLayer, //�Ƿ�ͼ��
		MapXOp_PIL_IN,           //��������
		MapXOp_PIL_ON,           //��������
		MapXOp_PIL_OUT,          //��������
	}CMapXOpResult;//�������

	CMapXReplace(void){}
	/******************************************************************************
	**�������� �� �ڵ�ͼ�������layerNameΪ���Ƶ�ͼ��
	**����     ��
	              layerName �� ͼ����
				  layer     �� ͼ��
	**����     �� ��ӽ��
	******************************************************************************/
	CMapXOpResult AddLayer(std::string layerName, MapXLayer& layer);

	/******************************************************************************
	**�������� �� �ж�ָ���ĵ��Ƿ���ָ��ͼ����
	**����     ��
	              layerName �� ͼ����
				  point     �� Ҫ�жϵĵ�
	**����     �� �жϽ��
	******************************************************************************/
	CMapXOpResult IsPointInLayer(std::string layerName, MapXPoint& point);

	/******************************************************************************
	**�������� �� �ж�ָ���ĵ��Ƿ���ָ������
	**����     ��
	              layer     �� ͼ��
				  point     �� Ҫ�жϵĵ�
	**����     �� �жϽ��
	******************************************************************************/
	CMapXOpResult IsPointInLayer(MapXLayer& layer, MapXPoint& point);

	/******************************************************************************
	**�������� �� ��������GPS��֮��ľ���
	**����     ��
	              pt1       �� ��һ����
				  pt2       �� �ڶ�����
	**����     �� ��������ľ���
	******************************************************************************/
	double CetDistance(MapXPoint& pt1, MapXPoint& pt2);

private:
	std::map<std::string, HRGN> layers;

	inline long ImageToRegion(double d);
	HRGN MakeRangeByLayer(MapXLayer& layer);
	inline bool IsInLine(MapXPoint& pt1, MapXPoint& pt2, MapXPoint& pt3);
};

//extern MAPXREPLACE_API int nMapXReplace;

//MAPXREPLACE_API int fnMapXReplace(void);
