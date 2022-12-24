#include <opencv2/opencv.hpp>
#include <iostream>


float srgbToLinear(float x) {
  if (x <= 0.0f)
    return 0.0f;
  else if (x >= 1.0f)
    return 1.0f;
  else if (x < 0.04045f)
    return x / 12.92f;
  else
    return std::pow((x + 0.055f) / 1.055f, 2.4f);
}


cv::Mat convertToLinearRgb(cv::Mat& image) {
  cv::Mat linearRgb;
  image.convertTo(linearRgb, CV_32FC3, 1/255.0);
  for (auto it = linearRgb.begin<cv::Vec<float, 3>>(); it != linearRgb.end<cv::Vec<float, 3>>(); it++) {
    for (int ch = 0; ch < 3; ch++) {
      float currValue = (*it)[ch];
      float converted = srgbToLinear(currValue);
      (*it)[ch] = converted;
    }
  }
  return linearRgb;
}

cv::Mat convertToLBA(cv::Mat& red, cv::Mat& green, cv::Mat& blue) {
  cv::Mat lba(1, 256, CV_32FC3);
  for (int i = 0; i < red.rows; i ++) {
    float r = red.at<float>(i);
    float g = green.at<float>(i);
    float b = blue.at<float>(i);
    float alpha = ((r - g) / std::sqrt(2.0f));
    float beta =  ((2.0f * b) - r - g) / std::sqrt(6.0f);
    float l =  (r + g + b) / std::sqrt(3.0f);
    cv::Vec<float, 3> lbaVec(l, alpha, beta);
    lba.at<cv::Vec<float, 3>>(0, i) = lbaVec;
  }

  cv::normalize(lba, lba, 0, 255, cv::NORM_MINMAX, -1, cv::Mat());
  return lba;
}

cv::Mat unique(cv::Mat& img) {
  std::map<std::string, bool> pixelMap;
  cv::Mat uniquePixels = cv::Mat();
  for (int i = 0; i < img.rows; ++i) {
    for (int j = 0; j < img.cols; ++j) {
      auto current = img.at<cv::Vec3b>(i, j);
      auto firstValue = current[0];
      auto secondValue = current[1];
      auto thirdValue = current[2];
      std::string firstString = std::to_string(firstValue);
      std::string secondString = std::to_string(secondValue);
      std::string thirdString = std::to_string(thirdValue);
      std::string key = firstString + "_" + secondString + "_" + thirdString;
      if (pixelMap[key]) {
        continue;
      }
      pixelMap[key] = true;
      cv::Vec3b vector{ firstValue, secondValue, thirdValue };
      uniquePixels.push_back(vector);
    }
  }
  return uniquePixels;
}

class Comparison {
  std::string folderPath;
  std::string originalFileName;
  std::string copyFileName;
  cv::Mat originalRaw;
  cv::Mat copyRaw;
  cv::Mat originalLinRGB;
  cv::Mat copyLinRGB;
  cv::Mat originalHSV;
  cv::Mat copyHSV;
  std::string filename;

public:
  Comparison(std::string& folder, std::string& origName, std::string& copyName, std::string& jsonName) {
    folderPath = folder;
    originalFileName = origName;
    copyFileName = copyName;
    originalRaw = cv::imread(folder + origName);
    copyRaw = cv::imread(folder + copyName);
    filename = jsonName;
  }

  void toLinRgb() {
    originalLinRGB = convertToLinearRgb(originalRaw);
    copyLinRGB = convertToLinearRgb(copyRaw);
  }

  void saveDataToJson() {
    cv::FileStorage json = cv::FileStorage(filename, cv::FileStorage::Mode::WRITE);
    cv::Mat originalIntRgb;
    originalLinRGB.convertTo(originalIntRgb, CV_8UC3, 255);
    cv::Mat originalLinearPlanes[3];
    cv::split(unique(originalIntRgb), originalLinearPlanes);

    json << "original" << "{:";
    json << "linrgb" << "{:";
    json << "blue" << originalLinearPlanes[0];
    json << "green" << originalLinearPlanes[1];
    json << "red" << originalLinearPlanes[2];
    json << "}";
    json << "filename" << originalFileName;
    json <<  "}";

    cv::Mat copyIntRgb;
    copyLinRGB.convertTo(copyIntRgb, CV_8UC3, 255);
    cv::Mat copyLinearPlanes[3];
    cv::split(unique(copyIntRgb), copyLinearPlanes);

    json << "copy" << "{:";
    json << "linrgb" << "{:";
    json << "blue" << copyLinearPlanes[0];
    json << "green" << copyLinearPlanes[1];
    json << "red" << copyLinearPlanes[2];
    json << "}";
    json << "filename" << copyFileName;
    json <<  "}";

    json.release();
  }
};

int main() {
  std::vector<std::vector<std::string>> images {
//      {"100-copy.png", "100-orig.png"},
//      {"500-copy.png", "500-orig.png"},
      {"1000-copy.png", "1000-orig.png"},
  };

  std::string folder = "../data/comparison/punk/small/";

  for (int i = 0; i < images.size(); i++) {
    std::vector<std::string> imgPair = images[i];
    std::string copyFilename = imgPair[0];
    std::string origFilename = imgPair[1];
    std::string jsonName = "comparison_" + std::to_string(i) + ".json";
    Comparison comparison(folder, origFilename, copyFilename, jsonName);
    comparison.toLinRgb();
    comparison.saveDataToJson();
  }
  return 0;
}