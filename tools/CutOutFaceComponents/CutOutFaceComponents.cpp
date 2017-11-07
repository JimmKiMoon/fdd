#include<iostream>
#include<opencv2/opencv.hpp>
#include "LBFRegressor.h"

using namespace std;
using namespace cv;

//������������Ӿ����е�ͼ��
cv::Mat boundingBoxImg(cv::Mat & cImg, vector<cv::Point> &featurePoints, double scale = 1.0)
{
	//�õ���ת����Ӿ���
	cv::RotatedRect rotatedRect = cv::minAreaRect(featurePoints);
	//�Ŵ���ο�
	rotatedRect.size.height *= scale;
	rotatedRect.size.width *= scale;
	Point2f vertices[4];
	rotatedRect.points(vertices);
	//���ζ��������
	vector<cv::Point2f> rotatedVertices;
	for (int i = 0; i < 4; i++)
	{
		//line(cImg, vertices[i], vertices[(i + 1) % 4], Scalar(255));
		rotatedVertices.push_back(vertices[i]);
	}
	//���ζ������갴x��������
	std::sort(rotatedVertices.begin(), rotatedVertices.end(),
		[](const cv::Point2f p0, const cv::Point2f p1)
		-> bool
	{
		return p0.x < p1.x;
	});

	//�����νǶ���תͼ�� 
	cv::Rect rect;
	rect.width = rotatedRect.size.width;
	rect.height = rotatedRect.size.height;
	float rotatedAngle = rotatedRect.angle;
	if (abs(rotatedAngle)>45)
	{
		rotatedAngle += 90;
		rect.width = rotatedRect.size.height;
		rect.height = rotatedRect.size.width;
	}

	cv::Mat rotatedMat = cv::getRotationMatrix2D(rotatedRect.center, rotatedAngle, 1);
	cv::Mat cRotatedImg;
	cv::warpAffine(cImg, cRotatedImg, rotatedMat, cImg.size());
	//cv::imshow("rotatedImg", rotatedImg);
	cv::Mat rotatedMatf;
	rotatedMat.convertTo(rotatedMatf, CV_64FC1);
	cv::Point leftTopPoint = rotatedVertices[0].y < rotatedVertices[1].y ? rotatedVertices[0] : rotatedVertices[1];
	rect.x = rotatedMatf.at<double>(0, 0)*leftTopPoint.x + rotatedMatf.at<double>(0, 1)*leftTopPoint.y + rotatedMatf.at<double>(0, 2);
	rect.y = rotatedMatf.at<double>(1, 0)*leftTopPoint.x + rotatedMatf.at<double>(1, 1)*leftTopPoint.y + rotatedMatf.at<double>(1, 2);
	if (rect.x+rect.width>=cRotatedImg.size().width||rect.y+rect.height>=cRotatedImg.size().height) {
		return cv::Mat();
	}
	//����ת���ͼ���Ͻ�ȡˮƽ����ͼ��
	return cRotatedImg(rect);
}

//ɾ���ļ�����
void DeleteContents(const string & dir_name)
{
	string files = dir_name + "/*.*";
	CFileFind ff;
	BOOL bHasNext = ff.FindFile((LPCTSTR)files.c_str());
	while (bHasNext)
	{
		bHasNext = ff.FindNextFile();
		if (ff.IsDots())
		{
			continue;
		}
		else if (ff.IsDirectory())
		{
			DeleteContents((LPCTSTR)ff.GetFilePath());
			RemoveDirectory((LPCTSTR)ff.GetFilePath());
		}
		else
		{
			DeleteFile((LPCTSTR)ff.GetFilePath());
		}
	}
}

const string dataSetHome="C:/Users/lyfe667/Desktop/data_hsy";
int main()
{
	//�������������
	cv::CascadeClassifier fcc;
	fcc.load((modelPath + cascadeName).c_str());
	//����ȫ�ֲ���
	ReadGlobalParamFromFile(modelPath + "LBF.model");
	//����3000fpsģ���ļ�
	LBFRegressor regressor;
	regressor.Load(modelPath + "LBF.model");
	vector<string> datasets;
	//datasets.push_back("close_mouthes");
	datasets.push_back("completely_close_eyes");
	//datasets.push_back("completely_open_mouthes");
	datasets.push_back("open_eyes");
	for (int i = 0;i < datasets.size();i++) {
		CFileFind ff;
		BOOL bHasNext = ff.FindFile((dataSetHome + "/" + datasets[i] + "/*.jpg").c_str());
		int imgCount = 0;
		while (bHasNext)
		{
			bHasNext = ff.FindNextFile();
			if (!ff.IsDirectory())
			{
				allPoints.clear();
				cout << ff.GetFilePath().GetBuffer() << endl;
				cImg = cv::imread(ff.GetFilePath().GetBuffer());
				cv::cvtColor(cImg, gImg, CV_BGR2GRAY);
				cv::resize(gImg, gSmallImgForFaceDetection, cv::Size(scaleForFaceDetection*gImg.size().width, scaleForFaceDetection*gImg.size().height));
				cv::equalizeHist(gSmallImgForFaceDetection, gSmallImgForFaceDetection);
				cout << gSmallImgForFaceDetection.size() << endl;
				clock_t begin = clock();
				fcc.detectMultiScale(gSmallImgForFaceDetection, faces,
					1.1, 2, 0
					//|CV_HAAR_FIND_BIGGEST_OBJECT
					//|CV_HAAR_DO_ROUGH_SEARCH
					| CV_HAAR_SCALE_IMAGE
					,
					cv::Size(10, 10));
				clock_t end = clock();
				cout << static_cast<float>(end - begin) / CLOCKS_PER_SEC << endl;
				if (!faces.empty())
				{
					//������������
					sort(faces.begin(), faces.end(),
						[](const cv::Rect &face0, const cv::Rect &face1)
						-> bool
					{
						return face0.size().area() > face1.size().area();
					});
					cv::Rect face = faces[0];
					//cv::rectangle(cImg, cv::Rect(face.x / scaleForFaceDetection, face.y / scaleForFaceDetection, face.width / scaleForFaceDetection, face.height / scaleForFaceDetection), cv::Scalar(0, 255, 255), 4);
					BoundingBox boundingbox;
					boundingbox.start_x = face.x;
					boundingbox.start_y = face.y;
					boundingbox.width = (face.width - 1);
					boundingbox.height = (face.height - 1);
					boundingbox.centroid_x = boundingbox.start_x + boundingbox.width / 2.0;
					boundingbox.centroid_y = boundingbox.start_y + boundingbox.height / 2.0;
					Mat_<double> current_shape = regressor.Predict(gSmallImgForFaceDetection, boundingbox, 1);
					for (int i = 0;i < global_params.landmark_num;i++) {
						//circle(cImg, Point2d(current_shape(i, 0) / scaleForFaceDetection, current_shape(i, 1) / scaleForFaceDetection), 2, Scalar(0, 255, 0), -1, 8, 0);
						//��������������
						allPoints.push_back(cv::Point(current_shape(i, 0) / scaleForFaceDetection, current_shape(i, 1) / scaleForFaceDetection));
						/*char buf[8];
						sprintf(buf, "%d", i);
						putText(cImg, buf, cv::Point(current_shape(i, 0) * 5, current_shape(i, 1) * 5), 0, 0.5, cv::Scalar(255, 0, 0));*/
					}
					vector<cv::Point> rightEyePoints;
					rightEyePoints.push_back(allPoints[4]);
					rightEyePoints.push_back(allPoints[5]);
					rightEyePoints.push_back(allPoints[6]);
					rightEyePoints.push_back(allPoints[7]);
					rightEyePoints.push_back(allPoints[8]);
					rightEyePoints.push_back(allPoints[9]);
					//���������Ӿ���ͼ��
					cv::Mat cRightEyeImg = boundingBoxImg(cImg, rightEyePoints, eyeRectScale);
					if (cRightEyeImg.size().area()==0) {
						continue;
					}
					if (!PathIsDirectory((dataSetHome+"/cut_right_eye_" + datasets[i]).c_str())) {
						CreateDirectory((dataSetHome+"/cut_right_eye_" + datasets[i]).c_str(), NULL);
					}
					time_t now = time(0);
					char rightEyeFileName[256];
					sprintf(rightEyeFileName,"%s/cr%d%d.jpg",(dataSetHome+"/cut_right_eye_"+datasets[i]).c_str(),now,imgCount);
					cv::imwrite(rightEyeFileName,cRightEyeImg);
					vector<cv::Point> leftEyePoints;
					leftEyePoints.push_back(allPoints[10]);
					leftEyePoints.push_back(allPoints[11]);
					leftEyePoints.push_back(allPoints[12]);
					leftEyePoints.push_back(allPoints[13]);
					leftEyePoints.push_back(allPoints[14]);
					leftEyePoints.push_back(allPoints[15]);
					cv::Mat cLeftEyeImg = boundingBoxImg(cImg, leftEyePoints, eyeRectScale);
					if (cLeftEyeImg.size().area()==0) {
						continue;
					}
					if (!PathIsDirectory((dataSetHome+"/cut_left_eye_" + datasets[i]).c_str())) {
						CreateDirectory((dataSetHome+"/cut_left_eye_" + datasets[i]).c_str(), NULL);
					}
					now = time(0);
					char leftEyeFileName[256];
					sprintf(leftEyeFileName, "%s/cl%d%d.jpg", (dataSetHome+"/cut_left_eye_"+datasets[i]).c_str(), now, imgCount);
					cv::imwrite(leftEyeFileName, cLeftEyeImg);
					std::vector<cv::Point> mouthChinPoints;
					mouthChinPoints.push_back(allPoints[1]);
					mouthChinPoints.push_back(allPoints[2]);
					mouthChinPoints.push_back(allPoints[16]);
					mouthChinPoints.push_back(allPoints[17]);
					mouthChinPoints.push_back(allPoints[18]);
					mouthChinPoints.push_back(allPoints[19]);
					cv::Mat cMouthImg = boundingBoxImg(cImg,mouthChinPoints,1);
					if (cMouthImg.size().area()==0) {
						continue;
					}
					if (!PathIsDirectory((dataSetHome+"/cut_mouth_" + datasets[i]).c_str())) {
						CreateDirectory((dataSetHome+"/cut_mouth_" + datasets[i]).c_str(),NULL);
					}
					now = time(0);
					char mouthFileName[256];
					sprintf(mouthFileName, "%s/cm%d%d.jpg", (dataSetHome+"/cut_mouth_" + datasets[i]).c_str(), now, imgCount);
					cv::imwrite(mouthFileName,cMouthImg);
					//cv::imshow("leftEyeImg", cLeftEyeImg);
					cv::imshow("cImg", cImg);
					cv::waitKey(1);
					imgCount++;
				}
			}
		}
	}	
	return 0;
}
