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
		double longitude;//经度
		double latitude;//纬度
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
	};//图层

	typedef enum
	{
		MapXOp_Init,             //初始值，无意义
		MapXOp_Para_Err,         //参数错误
		MapXOp_Add_Fail,         //添加图层失败
		MapXOp_Add_Exist,        //添加图层失败，已经存在
		MapXOp_Add_Succ,         //添加图层成功
		MapXOp_PIL_InvalidLayer, //非法图层
		MapXOp_PIL_IN,           //在区域内
		MapXOp_PIL_ON,           //在区域上
		MapXOp_PIL_OUT,          //在区域外
	}CMapXOpResult;//操作结果

	CMapXReplace(void){}
	/******************************************************************************
	**函数功能 ： 在地图中添加以layerName为名称的图层
	**参数     ：
	              layerName ： 图层名
				  layer     ： 图层
	**返回     ： 添加结果
	******************************************************************************/
	CMapXOpResult AddLayer(std::string layerName, MapXLayer& layer);

	/******************************************************************************
	**函数功能 ： 判断指定的点是否在指定图层内
	**参数     ：
	              layerName ： 图层名
				  point     ： 要判断的点
	**返回     ： 判断结果
	******************************************************************************/
	CMapXOpResult IsPointInLayer(std::string layerName, MapXPoint& point);

	/******************************************************************************
	**函数功能 ： 判断指定的点是否在指区域内
	**参数     ：
	              layer     ： 图层
				  point     ： 要判断的点
	**返回     ： 判断结果
	******************************************************************************/
	CMapXOpResult IsPointInLayer(MapXLayer& layer, MapXPoint& point);

	/******************************************************************************
	**函数功能 ： 计算两个GPS点之间的距离
	**参数     ：
	              pt1       ： 第一个点
				  pt2       ： 第二个点
	**返回     ： 上述两点的距离
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
