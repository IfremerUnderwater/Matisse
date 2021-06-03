#ifndef MATISSE_POINT_CLOUD_DENSIFY_INTERFACE_MVG_MVS_H_
#define MATISSE_POINT_CLOUD_DENSIFY_INTERFACE_MVG_MVS_H_


// I N C L U D E S /////////////////////////////////////////////////

#include "openMVG/cameras/Camera_Pinhole.hpp"
#include "openMVG/cameras/Camera_undistort_image.hpp"
#include "openMVG/image/image_io.hpp"
#include "openMVG/sfm/sfm_data.hpp"
#include "openMVG/sfm/sfm_data_io.hpp"

#include "third_party/cmdLine/cmdLine.h"
#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"
#include "third_party/progress/progress_display.hpp"

#include <atomic>
#include <cstdlib>
#include <string>

#ifdef OPENMVG_USE_OPENMP
#include <omp.h>
#endif

#include <fstream>

// D E F I N E S ///////////////////////////////////////////////////

#define MVSI_PROJECT_ID "MVSI" // identifies the project stream
#define MVSI_PROJECT_VER ((uint32_t)2) // identifies the version of a project stream

// set a default namespace name is none given
#ifndef _INTERFACE_NAMESPACE
#define _INTERFACE_NAMESPACE LOCAL_MVS
#endif

#define _USE_EIGEN
#define _USE_CUSTOM_CV

#ifndef NO_ID
#define NO_ID std::numeric_limits<uint32_t>::max()
#endif



// S T R U C T S ///////////////////////////////////////////////////

#ifdef _USE_CUSTOM_CV

namespace eigen_cv {

// simple cv::Matx
template<typename Type, int m, int n>
class Matx
{
public:
	typedef Type value_type;
	inline Matx() {}
	#ifdef _USE_EIGEN
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW_IF_VECTORIZABLE_FIXED_SIZE(Type,m*n)
	typedef Eigen::Matrix<Type,m,n,(n>1?Eigen::RowMajor : Eigen::ColMajor)> EMat;
	typedef Eigen::Map<const EMat> CEMatMap;
	typedef Eigen::Map<EMat> EMatMap;
	template<typename Derived>
	inline Matx(const Eigen::DenseBase<Derived>& rhs) { operator EMatMap () = rhs; }
	template<typename Derived>
	inline Matx& operator = (const Eigen::DenseBase<Derived>& rhs) { operator EMatMap () = rhs; return *this; }
	inline operator CEMatMap() const { return CEMatMap((const Type*)val); }
	inline operator EMatMap () { return EMatMap((Type*)val); }
	#endif
	static Matx eye() {
		Matx M;
		memset(M.val, 0, sizeof(Type)*m*n);
		const int shortdim(m < n ? m : n);
		for (int i = 0; i < shortdim; i++)
			M(i,i) = 1;
		return M;
	}
	Type operator()(int r, int c) const { return val[r*n+c]; }
	Type& operator()(int r, int c) { return val[r*n+c]; }
public:
	Type val[m*n];
};

// simple cv::Matx
template<typename Type>
class Point3_
{
public:
	typedef Type value_type;
	inline Point3_() {}
	#ifdef _USE_EIGEN
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW_IF_VECTORIZABLE_FIXED_SIZE(Type,3)
	typedef Eigen::Matrix<Type,3,1> EVec;
	typedef Eigen::Map<EVec> EVecMap;
	template<typename Derived>
	inline Point3_(const Eigen::DenseBase<Derived>& rhs) { operator EVecMap () = rhs; }
	template<typename Derived>
	inline Point3_& operator = (const Eigen::DenseBase<Derived>& rhs) { operator EVecMap () = rhs; return *this; }
	inline operator const EVecMap () const { return EVecMap((Type*)this); }
	inline operator EVecMap () { return EVecMap((Type*)this); }
	#endif
public:
	Type x, y, z;
};

} // namespace cv
#endif
/*----------------------------------------------------------------*/

std::string swapExtToPng(std::string _in_file)
{
	return stlplus::basename_part(_in_file) + ".png";
}

namespace _INTERFACE_NAMESPACE {

// custom serialization
namespace ARCHIVE {

// Basic serialization types
struct ArchiveSave {
	std::ostream& stream;
	uint32_t version;
	ArchiveSave(std::ostream& _stream, uint32_t _version)
		: stream(_stream), version(_version) {}
	template<typename _Tp>
	ArchiveSave& operator & (const _Tp& obj);
};
struct ArchiveLoad {
	std::istream& stream;
	uint32_t version;
	ArchiveLoad(std::istream& _stream, uint32_t _version)
		: stream(_stream), version(_version) {}
	template<typename _Tp>
	ArchiveLoad& operator & (_Tp& obj);
};

template<typename _Tp>
bool Save(ArchiveSave& a, const _Tp& obj) {
	const_cast<_Tp&>(obj).serialize(a, a.version);
	return true;
}
template<typename _Tp>
bool Load(ArchiveLoad& a, _Tp& obj) {
	obj.serialize(a, a.version);
	return true;
}

template<typename _Tp>
ArchiveSave& ArchiveSave::operator & (const _Tp& obj) {
	Save(*this, obj);
	return *this;
}
template<typename _Tp>
ArchiveLoad& ArchiveLoad::operator & (_Tp& obj) {
	Load(*this, obj);
	return *this;
}

// Main exporter & importer
template<typename _Tp>
bool SerializeSave(const _Tp& obj, const std::string& fileName, uint32_t version=MVSI_PROJECT_VER) {
	// open the output stream
	std::ofstream stream(fileName, std::ofstream::binary);
	if (!stream.is_open())
		return false;
	// write header
	if (version > 0) {
		// save project ID
		stream.write(MVSI_PROJECT_ID, 4);
		// save project version
		stream.write((const char*)&version, sizeof(uint32_t));
		// reserve some bytes
		const uint32_t reserved(0);
		stream.write((const char*)&reserved, sizeof(uint32_t));
	}
	// serialize out the current state
	ARCHIVE::ArchiveSave serializer(stream, version);
	serializer & obj;
	return true;
}
template<typename _Tp>
bool SerializeLoad(_Tp& obj, const std::string& fileName, uint32_t* pVersion=NULL) {
	// open the input stream
	std::ifstream stream(fileName, std::ifstream::binary);
	if (!stream.is_open())
		return false;
	// read header
	uint32_t version(0);
	// load project header ID
	char szHeader[4];
	stream.read(szHeader, 4);
	if (!stream)
		return false;
	if (strncmp(szHeader, MVSI_PROJECT_ID, 4) != 0) {
		// try to load as the first version that didn't have a header
		const size_t size(fileName.size());
		if (size <= 4)
			return false;
		std::string ext(fileName.substr(size-4));
		std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
		if (ext != ".mvs")
			return false;
		stream.seekg(0, std::ifstream::beg);
	} else {
		// load project version
		stream.read((char*)&version, sizeof(uint32_t));
		if (!stream || version > MVSI_PROJECT_VER)
			return false;
		// skip reserved bytes
		uint32_t reserved;
		stream.read((char*)&reserved, sizeof(uint32_t));
	}
	// serialize in the current state
	ARCHIVE::ArchiveLoad serializer(stream, version);
	serializer & obj;
	if (pVersion)
		*pVersion = version;
	return true;
}


#define ARCHIVE_DEFINE_TYPE(TYPE) \
template<> \
bool Save<TYPE>(ArchiveSave& a, const TYPE& v) { \
	a.stream.write((const char*)&v, sizeof(TYPE)); \
	return true; \
} \
template<> \
bool Load<TYPE>(ArchiveLoad& a, TYPE& v) { \
	a.stream.read((char*)&v, sizeof(TYPE)); \
	return true; \
}

// Serialization support for basic types
ARCHIVE_DEFINE_TYPE(uint32_t)
ARCHIVE_DEFINE_TYPE(uint64_t)
ARCHIVE_DEFINE_TYPE(float)
ARCHIVE_DEFINE_TYPE(double)
#ifdef __APPLE__
  ARCHIVE_DEFINE_TYPE(unsigned long)
#endif

// Serialization support for cv::Matx
template<typename _Tp, int m, int n>
bool Save(ArchiveSave& a, const eigen_cv::Matx<_Tp,m,n>& _m) {
	a.stream.write((const char*)_m.val, sizeof(_Tp)*m*n);
	return true;
}
template<typename _Tp, int m, int n>
bool Load(ArchiveLoad& a, eigen_cv::Matx<_Tp,m,n>& _m) {
	a.stream.read((char*)_m.val, sizeof(_Tp)*m*n);
	return true;
}

// Serialization support for cv::Point3_
template<typename _Tp>
bool Save(ArchiveSave& a, const eigen_cv::Point3_<_Tp>& pt) {
	a.stream.write((const char*)&pt.x, sizeof(_Tp)*3);
	return true;
}
template<typename _Tp>
bool Load(ArchiveLoad& a, eigen_cv::Point3_<_Tp>& pt) {
	a.stream.read((char*)&pt.x, sizeof(_Tp)*3);
	return true;
}

// Serialization support for std::string
template<>
bool Save<std::string>(ArchiveSave& a, const std::string& s) {
	const size_t size(s.size());
	Save(a, size);
	if (size > 0)
		a.stream.write(&s[0], sizeof(char)*size);
	return true;
}
template<>
bool Load<std::string>(ArchiveLoad& a, std::string& s) {
	size_t size;
	Load(a, size);
	if (size > 0) {
		s.resize(size);
		a.stream.read(&s[0], sizeof(char)*size);
	}
	return true;
}

// Serialization support for std::vector
template<typename _Tp>
bool Save(ArchiveSave& a, const std::vector<_Tp>& v) {
	const size_t size(v.size());
	Save(a, size);
	for (size_t i=0; i<size; ++i)
		Save(a, v[i]);
	return true;
}
template<typename _Tp>
bool Load(ArchiveLoad& a, std::vector<_Tp>& v) {
	size_t size;
	Load(a, size);
	if (size > 0) {
		v.resize(size);
		for (size_t i=0; i<size; ++i)
			Load(a, v[i]);
	}
	return true;
}

} // namespace ARCHIVE
/*----------------------------------------------------------------*/


// interface used to export/import MVS input data;
//  - MAX(width,height) is used for normalization
//  - row-major order is used for storing the matrices
struct Interface
{
	typedef eigen_cv::Point3_<float> Pos3f;
	typedef eigen_cv::Point3_<double> Pos3d;
	typedef eigen_cv::Matx<double,3,3> Mat33d;
	typedef eigen_cv::Matx<double,4,4> Mat44d;
	typedef eigen_cv::Point3_<uint8_t> Col3; // x=B, y=G, z=R
	/*----------------------------------------------------------------*/

	// structure describing a mobile platform with cameras attached to it
	struct Platform {
		// structure describing a camera mounted on a platform
		struct Camera {
			std::string name; // camera's name
			uint32_t width, height; // image resolution in pixels for all images sharing this camera (optional)
			Mat33d K; // camera's intrinsics matrix (normalized if image resolution not specified)
			Mat33d R; // camera's rotation matrix relative to the platform
			Pos3d C; // camera's translation vector relative to the platform

			Camera() : width(0), height(0) {}
			bool HasResolution() const { return width > 0 && height > 0; }
			bool IsNormalized() const { return !HasResolution(); }

			template <class Archive>
			void serialize(Archive& ar, const unsigned int version) {
				ar & name;
				if (version > 0) {
					ar & width;
					ar & height;
				}
				ar & K;
				ar & R;
				ar & C;
			}
		};
		typedef std::vector<Camera> CameraArr;

		// structure describing a pose along the trajectory of a platform
		struct Pose {
			Mat33d R; // platform's rotation matrix
			Pos3d C; // platform's translation vector in the global coordinate system

			template <class Archive>
			void serialize(Archive& ar, const unsigned int /*version*/) {
				ar & R;
				ar & C;
			}
		};
		typedef std::vector<Pose> PoseArr;

		std::string name; // platform's name
		CameraArr cameras; // cameras mounted on the platform
		PoseArr poses; // trajectory of the platform

		template <class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			ar & name;
			ar & cameras;
			ar & poses;
		}
	};
	typedef std::vector<Platform> PlatformArr;
	/*----------------------------------------------------------------*/

	// structure describing an image
	struct Image {
		std::string name; // image file name
		uint32_t platformID; // ID of the associated platform
		uint32_t cameraID; // ID of the associated camera on the associated platform
		uint32_t poseID; // ID of the pose of the associated platform

		template <class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			ar & name;
			ar & platformID;
			ar & cameraID;
			ar & poseID;
		}
	};
	typedef std::vector<Image> ImageArr;
	/*----------------------------------------------------------------*/

	// structure describing a 3D point
	struct Vertex {
		// structure describing one view for a given 3D feature
		struct View {
			uint32_t imageID; // image ID corresponding to this view
			float confidence; // view's confidence (0 - not available)

			template<class Archive>
			void serialize(Archive& ar, const unsigned int /*version*/) {
				ar & imageID;
				ar & confidence;
			}
		};
		typedef std::vector<View> ViewArr;

		Pos3f X; // 3D point position
		ViewArr views; // list of all available views for this 3D feature

		template <class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			ar & X;
			ar & views;
		}
	};
	typedef std::vector<Vertex> VertexArr;
	/*----------------------------------------------------------------*/

	// structure describing a 3D line
	struct Line {
		// structure describing one view for a given 3D feature
		struct View {
			uint32_t imageID; // image ID corresponding to this view
			float confidence; // view's confidence (0 - not available)

			template<class Archive>
			void serialize(Archive& ar, const unsigned int /*version*/) {
				ar & imageID;
				ar & confidence;
			}
		};
		typedef std::vector<View> ViewArr;

		Pos3f pt1; // 3D line segment end-point
		Pos3f pt2; // 3D line segment end-point
		ViewArr views; // list of all available views for this 3D feature

		template <class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			ar & pt1;
			ar & pt2;
			ar & views;
		}
	};
	typedef std::vector<Line> LineArr;
	/*----------------------------------------------------------------*/

	// structure describing a 3D point's normal (optional)
	struct Normal {
		Pos3f n; // 3D feature normal

		template <class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			ar & n;
		}
	};
	typedef std::vector<Normal> NormalArr;
	/*----------------------------------------------------------------*/

	// structure describing a 3D point's color (optional)
	struct Color {
		Col3 c; // 3D feature color

		template <class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			ar & c;
		}
	};
	typedef std::vector<Color> ColorArr;
	/*----------------------------------------------------------------*/

	PlatformArr platforms; // array of platforms
	ImageArr images; // array of images
	VertexArr vertices; // array of reconstructed 3D points
	NormalArr verticesNormal; // array of reconstructed 3D points' normal (optional)
	ColorArr verticesColor; // array of reconstructed 3D points' color (optional)
	LineArr lines; // array of reconstructed 3D lines
	NormalArr linesNormal; // array of reconstructed 3D lines' normal (optional)
	ColorArr linesColor; // array of reconstructed 3D lines' color (optional)
	Mat44d transform; // transformation used to convert from absolute to relative coordinate system (optional)

	Interface() : transform(Mat44d::eye()) {}

	template <class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		ar & platforms;
		ar & images;
		ar & vertices;
		ar & verticesNormal;
		ar & verticesColor;
		if (version > 0) {
			ar & lines;
			ar & linesNormal;
			ar & linesColor;
			if (version > 1) {
				ar & transform;
			}
		}
	}
};
/*----------------------------------------------------------------*/

} // namespace _INTERFACE_NAMESPACE


bool exportToOpenMVS(
  const openMVG::sfm::SfM_Data & sfm_data,
  const std::string & sOutFile,
  const std::string & sOutDir,
  const int iNumThreads = 0,
  C_Progress * my_progress_bar = nullptr
  )
{
  // Create undistorted images directory structure
  if (!stlplus::is_folder(sOutDir))
  {
    stlplus::folder_create(sOutDir);
    if (!stlplus::is_folder(sOutDir))
    {
      std::cerr << "Cannot access to one of the desired output directory" << std::endl;
      return false;
    }
  }

  // Export data :
  LOCAL_MVS::Interface scene;
  size_t nPoses(0);
  const uint32_t nViews((uint32_t)sfm_data.GetViews().size());

  my_progress_bar->restart(nViews, "- PROCESS VIEWS -");

  // OpenMVG can have not contiguous index, use a map to create the required OpenMVS contiguous ID index
  std::map<openMVG::IndexT, uint32_t> map_intrinsic, map_view;

  // define a platform with all the intrinsic group
  for (const auto& intrinsic: sfm_data.GetIntrinsics())
  {
    if (isPinhole(intrinsic.second->getType()))
    {
      const openMVG::cameras::Pinhole_Intrinsic * cam = dynamic_cast<const openMVG::cameras::Pinhole_Intrinsic*>(intrinsic.second.get());
      if (map_intrinsic.count(intrinsic.first) == 0)
        map_intrinsic.insert(std::make_pair(intrinsic.first, scene.platforms.size()));
	  LOCAL_MVS::Interface::Platform platform;
      // add the camera
	  LOCAL_MVS::Interface::Platform::Camera camera;
      camera.K = cam->K();
      // sub-pose
      camera.R = openMVG::Mat3::Identity();
      camera.C = openMVG::Vec3::Zero();
      platform.cameras.push_back(camera);
      scene.platforms.push_back(platform);
    }
  }

  // define images & poses
  scene.images.reserve(nViews);
  for (const auto& view : sfm_data.GetViews())
  {
    map_view[view.first] = scene.images.size();
	LOCAL_MVS::Interface::Image image;
    const std::string srcImage = stlplus::create_filespec(sfm_data.s_root_path, view.second->s_Img_path);
    image.name = stlplus::create_filespec(sOutDir, swapExtToPng(view.second->s_Img_path)); // output png better dark value handling with undist img (not lossy)
    image.platformID = map_intrinsic.at(view.second->id_intrinsic);
	LOCAL_MVS::Interface::Platform& platform = scene.platforms[image.platformID];
    image.cameraID = 0;
    if (!stlplus::is_file(srcImage))
    {
      std::cout << "Cannot read the corresponding image: " << srcImage << std::endl;
      return EXIT_FAILURE;
    }
    if (sfm_data.IsPoseAndIntrinsicDefined(view.second.get()))
    {
      LOCAL_MVS::Interface::Platform::Pose pose;
      image.poseID = platform.poses.size();
      const openMVG::geometry::Pose3 poseMVG(sfm_data.GetPoseOrDie(view.second.get()));
      pose.R = poseMVG.rotation();
      pose.C = poseMVG.center();
      platform.poses.push_back(pose);
      ++nPoses;
    }
    else
    {
      // image have not valid pose, so set an undefined pose
      image.poseID = NO_ID;
      // just copy the image
      //stlplus::file_copy(srcImage, image.name);
    }
    scene.images.emplace_back(image);
    ++(*my_progress_bar);
  }

  // Export undistorted images
  my_progress_bar->restart(sfm_data.views.size(), "- UNDISTORT IMAGES -");

  std::atomic<bool> bOk(true); // Use a boolean to track the status of the loop process
#ifdef OPENMVG_USE_OPENMP
  const unsigned int nb_max_thread = (iNumThreads > 0)? iNumThreads : omp_get_max_threads();

  #pragma omp parallel for schedule(dynamic) num_threads(nb_max_thread)
#endif
  for (int i = 0; i < static_cast<int>(sfm_data.views.size()); ++i)
  {
    ++(*my_progress_bar);

    if (!bOk)
      continue;

	openMVG::sfm::Views::const_iterator iterViews = sfm_data.views.begin();
    std::advance(iterViews, i);
    const openMVG::sfm::View * view = iterViews->second.get();

    // Get image paths
    const std::string srcImage = stlplus::create_filespec(sfm_data.s_root_path, view->s_Img_path);
    const std::string imageName = stlplus::create_filespec(sOutDir, swapExtToPng(view->s_Img_path));

    if (!stlplus::is_file(srcImage))
    {
      std::cerr << "Cannot read the corresponding image: " << srcImage << std::endl;
      bOk = false;
      continue;
    }
    if (sfm_data.IsPoseAndIntrinsicDefined(view))
    {
      // export undistorted images
      const openMVG::cameras::IntrinsicBase * cam = sfm_data.GetIntrinsics().at(view->id_intrinsic).get();
      if (cam->have_disto())
      {
        // undistort image and save it
		openMVG::image::Image<openMVG::image::RGBColor> imageRGB, imageRGB_ud;
		openMVG::image::Image<uint8_t> image_gray, image_gray_ud;
        try
        {
          if (ReadImage(srcImage.c_str(), &imageRGB))
          {
            UndistortImage(imageRGB, cam, imageRGB_ud, openMVG::image::BLACK);
            bOk = WriteImage(imageName.c_str(), imageRGB_ud);
          }
          else // If RGBColor reading fails, try to read as gray image
          if (ReadImage(srcImage.c_str(), &image_gray))
          {
            UndistortImage(image_gray, cam, image_gray_ud, openMVG::image::BLACK);
            const bool bRes = WriteImage(imageName.c_str(), image_gray_ud);
            bOk = bOk & bRes;
          }
          else
          {
            bOk = false;
          }
        }
        catch (const std::bad_alloc& e)
        {
          bOk = false;
        }
      }
      else
      {
        // just copy image
        stlplus::file_copy(srcImage, imageName);
      }
    }
    else
    {
      // just copy the image
      stlplus::file_copy(srcImage, imageName);
    }
  }

  if (!bOk)
  {
    std::cerr << "Catched a memory error in the image conversion."
     << " Please consider to use less threads ([-n|--numThreads])." << std::endl;
    return EXIT_FAILURE;
  }

  // define structure
  scene.vertices.reserve(sfm_data.GetLandmarks().size());
  for (const auto& vertex: sfm_data.GetLandmarks())
  {
    const openMVG::sfm::Landmark & landmark = vertex.second;
	LOCAL_MVS::Interface::Vertex vert;
	LOCAL_MVS::Interface::Vertex::ViewArr& views = vert.views;
    for (const auto& observation: landmark.obs)
    {
      const auto it(map_view.find(observation.first));
      if (it != map_view.end()) {
		  LOCAL_MVS::Interface::Vertex::View view;
        view.imageID = it->second;
        view.confidence = 0;
        views.push_back(view);
      }
    }
    if (views.size() < 2)
      continue;
    std::sort(
      views.begin(), views.end(),
      [] (const LOCAL_MVS::Interface::Vertex::View& view0, const LOCAL_MVS::Interface::Vertex::View& view1)
      {
        return view0.imageID < view1.imageID;
      }
    );
    vert.X = landmark.X.cast<float>();
    scene.vertices.push_back(vert);
  }

  // normalize camera intrinsics
  for (size_t p=0; p<scene.platforms.size(); ++p)
  {
	  LOCAL_MVS::Interface::Platform& platform = scene.platforms[p];
    for (size_t c=0; c<platform.cameras.size(); ++c) {
		LOCAL_MVS::Interface::Platform::Camera& camera = platform.cameras[c];
      // find one image using this camera
		LOCAL_MVS::Interface::Image* pImage(nullptr);
      for (LOCAL_MVS::Interface::Image& image: scene.images)
      {
        if (image.platformID == p && image.cameraID == c && image.poseID != NO_ID)
        {
          pImage = &image;
          break;
        }
      }
      if (!pImage)
      {
        std::cerr << "error: no image using camera " << c << " of platform " << p << std::endl;
        continue;
      }
      // read image meta-data
	  openMVG::image::ImageHeader imageHeader;
      ReadImageHeader(pImage->name.c_str(), &imageHeader);
      const double fScale(1.0/std::max(imageHeader.width, imageHeader.height));
      camera.K(0, 0) *= fScale;
      camera.K(1, 1) *= fScale;
      camera.K(0, 2) *= fScale;
      camera.K(1, 2) *= fScale;
    }
  }

  // write OpenMVS data
  if (!LOCAL_MVS::ARCHIVE::SerializeSave(scene, sOutFile))
    return false;

  std::cout
    << "Scene saved to OpenMVS interface format:\n"
    << " #platforms: " << scene.platforms.size() << std::endl;
    for (int i = 0; i < scene.platforms.size(); ++i)
    {
      std::cout << "  platform ( " << i << " ) #cameras: " << scene.platforms[i].cameras.size() << std::endl;
    }
  std::cout
    << "  " << scene.images.size() << " images (" << nPoses << " calibrated)\n"
    << "  " << scene.vertices.size() << " Landmarks\n";
  return true;
}

#endif // MATISSE_POINT_CLOUD_DENSIFY_INTERFACE_MVG_MVS_H_
