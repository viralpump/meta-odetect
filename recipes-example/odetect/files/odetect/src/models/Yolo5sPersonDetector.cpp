#include "models/Yolo5sPersonDetector.hpp"

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

const std::string Yolo5sPersonDetector::modelName = "yolov5s-face.onnx";
using namespace std;
using namespace cv;

static inline float sigmoid_x(float x)
{
	return static_cast<float>(1.f / (1.f + exp(-x)));
}

Yolo5sPersonDetector::Yolo5sPersonDetector(const std::string& modelDir, const ODCaps inCaps, const void* modelData) 
    : IModelDnnDetector(inCaps)
{
    std::string modelPath = modelDir + "/" + modelName;
    net = cv::dnn::readNetFromONNX(modelPath);

    net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    bufferSize = inCaps.width * inCaps.height * inCaps.channels;
    buffer = new uint8_t[bufferSize];

	float conf = *static_cast<const float*>(modelData);
    objThreshold = conf > 0 && conf <= 1 ? conf : modelThDefault;
	confThreshold = objThreshold;
}

std::unique_ptr<IModelDnnDetector> Yolo5sPersonDetector::Construct(const std::string& modelDir, const ODCaps inCaps, const void* modelData) {
    return std::make_unique<Yolo5sPersonDetector>(modelDir, inCaps, modelData);
}

Yolo5sPersonDetector::~Yolo5sPersonDetector() {
    delete[] buffer;
}

void Yolo5sPersonDetector::Sigmoid(cv::Mat* out, int length)
{
	float* pdata = (float*)(out->data);
	int i = 0; 
	for (i = 0; i < length; i++) {
		pdata[i] = 1.0 / (1 + expf(-pdata[i]));
	}
}

void Yolo5sPersonDetector::drawPred(float conf, int left, int top, int right, int bottom, Mat& frame, std::vector<int> landmark) const
{
	//Draw a rectangle displaying the bounding box
	cv::rectangle(frame, Point(left, top), Point(right, bottom), Scalar(0, 255, 0), 3);
	
    //Draw key points
    for (int i = 0; i < 5; i++) {
		cv::circle(frame, cv::Point(landmark[2 * i], landmark[2 * i + 1]), 5, cv::Scalar(0, 0, 255), -1);
	}
}

bool Yolo5sPersonDetector::Detect(const OdBuf inBuf, OdBuf outBuf) const {
    memcpy(buffer, inBuf, bufferSize);

    cv::Mat bgrFrame;
    InputPreProcess(buffer, bgrFrame);

	cv::Mat input_blob = cv::dnn::blobFromImage(bgrFrame, 1 / 255.0, cv::Size(m_width, m_height), cv::Scalar(0, 0, 0), true, false);
	net.setInput(input_blob);

	std::vector<cv::Mat> outs;
	this->net.forward(outs, this->net.getUnconnectedOutLayersNames());

	std::vector<float> confidences;
	std::vector<Rect> boxes;
	std::vector<std::vector<int>> landmarks;
	float ratioh = (float)bgrFrame.rows / m_height, ratiow = (float)bgrFrame.cols / m_width;
	int n = 0, q = 0, i = 0, j = 0, nout = 16, row_ind = 0, k = 0; ///xmin,ymin,xamx,ymax,box_score,x1,y1, ... ,x5,y5,face_score
	for (n = 0; n < 3; n++) {
		int num_grid_x = (int)(m_width / this->stride[n]);
		int num_grid_y = (int)(m_height / this->stride[n]);
		for (q = 0; q < 3; q++) {
			const float anchor_w = this->anchors[n][q * 2];
			const float anchor_h = this->anchors[n][q * 2 + 1];
			for (i = 0; i < num_grid_y; i++) {
				for (j = 0; j < num_grid_x; j++) {
					float* pdata = (float*)outs[0].data + row_ind * nout;
					float box_score = sigmoid_x(pdata[4]);
					if (box_score > objThreshold) {
						float face_score = sigmoid_x(pdata[15]);
						// if (face_score > confThreshold) { 
							float cx = (sigmoid_x(pdata[0]) * 2.f - 0.5f + j) * this->stride[n];  ///cx
							float cy = (sigmoid_x(pdata[1]) * 2.f - 0.5f + i) * this->stride[n];   ///cy
							float w = powf(sigmoid_x(pdata[2]) * 2.f, 2.f) * anchor_w;   ///w
							float h = powf(sigmoid_x(pdata[3]) * 2.f, 2.f) * anchor_h;  ///h

							int left = (cx - 0.5*w)*ratiow;
							int top = (cy - 0.5*h)*ratioh;   

							confidences.push_back(face_score);
							boxes.push_back(Rect(left, top, (int)(w*ratiow), (int)(h*ratioh)));
							std::vector<int> landmark(10);
							for (k = 5; k < 15; k+=2) {
								const int ind = k - 5;
								landmark[ind] = (int)(pdata[k] * anchor_w + j * this->stride[n])*ratiow;
								landmark[ind + 1] = (int)(pdata[k + 1] * anchor_h + i * this->stride[n])*ratioh;
							}
							landmarks.push_back(landmark);
						// }
					}
					row_ind++;
				}
			}
		}
	}

	std::vector<int> indices;
	cv::dnn::NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);
	for (size_t i = 0; i < indices.size(); ++i) {
		int idx = indices[i];
		cv::Rect box = boxes[idx];

		drawPred(confidences[idx], box.x, box.y,
			box.x + box.width, box.y + box.height, bgrFrame, landmarks[idx]);

	}


    memcpy(outBuf, bgrFrame.data, inCaps.width * inCaps.height * 3);

    return true;
}