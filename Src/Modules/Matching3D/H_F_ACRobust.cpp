#include "H_F_ACRobust.hpp"

using namespace openMVG;
using namespace matching_image_collection;

// Definition of static variable
std::mutex GeometricFilter_H_F_AC::m_sift_gpu_mutex;