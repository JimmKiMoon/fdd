#pragma once
#include"Frame.h"
//#include"FaceAnalysisModel.h"
class FaceAnalysisModel;
class FaceComponent {
public:
	 enum Status {open,close};
protected:
	//ԭʼ֡
	Frame *pFrame_;
	//����ģ��
	FaceAnalysisModel *pModel_;
	//���������������
	std::vector<cv::Point> featurePoints_;
	//�����ͼ��
	cv::Mat colorImg_;
	//���ͼ���������
	double colorImgScale_;
	//���״̬
	Status status_;
	//�����Ӿ��εĶ���
	cv::Point2f vertices_[4];
public:
	FaceComponent();
    FaceComponent(const cv::Ptr<Frame> &pFrame,const cv::Ptr<FaceAnalysisModel> &pModel,double colorImgScale=1.0);
	virtual ~FaceComponent();
	//�������ͼ��
	void generateColorImg();
	//ģ�淽��
	FaceComponent::Status analyzeStatus();
	FaceComponent::Status status();
    void pFrame(const cv::Ptr<Frame> &pFrame);
    void pModel(const cv::Ptr<FaceAnalysisModel> &pModel);
	void featurePoints(std::vector<cv::Point> &featurePoints);
	void colorImgScale(double colorImgScale);
	//�����Ӿ��ζ���
	cv::Point2f * vertices();
	//������Ӿ���
	void drawMinAreaRect();
private:
	//Ԥ�����״̬
	virtual FaceComponent::Status predictStatus() = 0;
};
