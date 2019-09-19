//文件扫描程序：能够对纸质文件进行扫描处理，即图像的信息区域的提取与矫正
//2019/9/18

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include<algorithm>
using namespace cv;
using namespace std;


//角点排序
bool x_sort(const Point2f & m1, const Point2f & m2)
{
	return m1.x < m2.x;
}

//确定四个点的中心线
void sortCorners(vector<Point2f>& corners, Point2f center)
{
	vector<Point2f> top, bot;
	vector<Point2f> backup = corners;
	sort(corners.begin(), corners.end(), x_sort);  //注意先按x的大小给4个点排序

	for (int i = 0; i < corners.size(); i++)
	{
		if (corners[i].y < center.y && top.size() < 2)    //这里的小于2是为了避免三个顶点都在top的情况
			top.push_back(corners[i]);
		else
			bot.push_back(corners[i]);
	}
	corners.clear();

	if (top.size() == 2 && bot.size() == 2)
	{
		//cout << "log" << endl;
		cv::Point2f tl = top[0].x > top[1].x ? top[1] : top[0];
		cv::Point2f tr = top[0].x > top[1].x ? top[0] : top[1];
		cv::Point2f bl = bot[0].x > bot[1].x ? bot[1] : bot[0];
		cv::Point2f br = bot[0].x > bot[1].x ? bot[0] : bot[1];


		corners.push_back(tl);
		corners.push_back(tr);
		corners.push_back(br);
		corners.push_back(bl);
	}
	else
	{
		corners = backup;
	}
}


//图像矫正后的高度与宽度计算
int CalcDstSize_height(const vector<cv::Point2f>& corners)
{
	int dst_hight;
	int h1 = sqrt((corners[0].x - corners[3].x)*(corners[0].x - corners[3].x) + (corners[0].y - corners[3].y)*(corners[0].y - corners[3].y));
	int h2 = sqrt((corners[1].x - corners[2].x)*(corners[1].x - corners[2].x) + (corners[1].y - corners[2].y)*(corners[1].y - corners[2].y));
	return dst_hight = MAX(h1, h2);
}
int CalcDstSize_width(const vector<cv::Point2f>& corners)
{
	int dst_width;
	int w1 = sqrt((corners[0].x - corners[1].x)*(corners[0].x - corners[1].x) + (corners[0].y - corners[1].y)*(corners[0].y - corners[1].y));
	int w2 = sqrt((corners[2].x - corners[3].x)*(corners[2].x - corners[3].x) + (corners[2].y - corners[3].y)*(corners[2].y - corners[3].y));
	return dst_width = MAX(w1, w2);
}


int main()
{
	Mat src = imread("1-123029001-OCR-AH-A01_100.jpg");
	Mat img = src.clone();

	//储存轮廓的变量
	vector<vector<Point> > contours;
	vector<vector<Point> > f_contours;
	Mat contour_dstimg = Mat::zeros(src.rows, src.cols, CV_8UC3);

	//计算角点时使用的变量
	Mat corner_img;
	Mat bkup = src.clone();
	vector<Point2f> corners;
	int maxCornerNumber = 4;
	int r = 4;
	double qualityLevel = 0.01;
	double minDistance = 10;
	RNG rng(12345);


	//矫正计算使用的变量
	Mat source = src.clone();
	int g_dst_hight;
	int g_dst_width;

	//图像太大，改变下尺寸
	resize(src, src, Size(), 0.5, 0.5);
	resize(source, source, Size(), 0.5, 0.5);
	resize(img, img, Size(), 0.5, 0.5);
	resize(bkup, bkup, Size(), 0.5, 0.5);

	imshow("原图", src); 
	cvtColor(img, img, CV_RGB2GRAY);   //二值化
	GaussianBlur(img, img, Size(5, 5), 0, 0);  //高斯滤波
	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3)); //获取自定义核，第一个参数MORPH_RECT表示矩形的卷积核，当然还可以选择椭圆形的、交叉型的													 //膨胀操作
	dilate(img, img, element);  //实现过程中发现，适当的膨胀很重要

	//选取信息区域轮廓
	Canny(img, img, 30, 120, 3);   //边缘提取
	findContours(img, f_contours, RETR_EXTERNAL, CHAIN_APPROX_NONE); //找轮廓
	//求出面积最大的轮廓
	int max_area = 0;
	int index;
	for (int i = 0; i < f_contours.size(); i++)
	{
		double tmparea = fabs(contourArea(f_contours[i]));
		if (tmparea > max_area)
		{
			index = i;
			max_area = tmparea;
		}
	}
	contours.push_back(f_contours[index]);
	drawContours(contour_dstimg, contours, -1, Scalar(255, 255, 255));

	//计算角点

	cvtColor(contour_dstimg, contour_dstimg, CV_RGB2GRAY);
	goodFeaturesToTrack(contour_dstimg, corners, maxCornerNumber, qualityLevel, minDistance);

	//确定中心点
	Point2f center;
	double center_x = (corners[0].x + corners[1].x + corners[2].x + corners[3].x) / corners.size();
	double center_y = (corners[0].y + corners[1].y + corners[2].y + corners[3].y) / corners.size();
	center = (Point(center_x, center_y));

	//角点排序
	sortCorners(corners, center);

	//矫正后图像的宽度和高度
	g_dst_hight = CalcDstSize_height(corners);  //最终图像的高度
	g_dst_width = CalcDstSize_width(corners); //最终图像的宽度

	//矫正变换
	Mat quad = Mat::zeros(g_dst_hight, g_dst_width, CV_8UC3);
	vector<Point2f> quad_pts;
	quad_pts.push_back(Point2f(0, 0));
	quad_pts.push_back(Point2f(quad.cols, 0));
	quad_pts.push_back(Point2f(quad.cols, quad.rows));
	quad_pts.push_back(Point2f(0, quad.rows));
	Mat transmtx = getPerspectiveTransform(corners, quad_pts);
	warpPerspective(source, quad, transmtx, quad.size());
	imshow("quadrilateral", quad);

	waitKey(0);
}


