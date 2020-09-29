#include "force_looking_down_gui.h"
#include "ui_force_looking_down_gui.h"
#include <QDir>
#include <QDirIterator>
#include <QString>
#include <QStringList>
#include <QFileDialog>
#include <QDebug>
#include <QFileInfo>

#include <Eigen/Dense>
#include "openMVG/sfm/sfm.hpp"

#include "openMVG/third_party/cmdLine/cmdLine.h"
#include "openMVG/third_party/stlplus3/filesystemSimplified/file_system.hpp"
#include "openMVG/geometry/rigid_transformation3D_srt.hpp"
#include "openMVG/geometry/Similarity3.hpp"
#include "openMVG/geometry/Similarity3_Kernel.hpp"


#include <string>
#include <vector>

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>

using namespace openMVG;
using namespace openMVG::sfm;

using namespace std;

bool FindRT
(
	const Mat& x1,
	const Mat& x2,
	Vec3* t,
	Mat3* R
)
{
	if (x1.cols() < 3 || x2.cols() < 3)
	{
		return false;
	}

	assert(3 == x1.rows());
	assert(3 <= x1.cols());
	assert(x1.rows() == x2.rows());
	assert(x1.cols() == x2.cols());

	// Get the transformation via Umeyama's least squares algorithm. This returns
	// a matrix of the form:
	// [ s * R t]
	// [ 0 1]
	// from which we can extract the rotation, and translation.
	const Eigen::Matrix4d transform = Eigen::umeyama(x1, x2, false);

	// Check critical cases
	*R = transform.topLeftCorner<3, 3>();
	if (R->determinant() < 0)
	{
		return false;
	}

	// Extract transformation parameters
	*t = transform.topRightCorner<3, 1>();

	return true;
}

ForceLookingDownGUI::ForceLookingDownGUI(QWidget* parent)
	: QMainWindow(parent)
	, ui(new Ui::ForceLookingDownGUI)
{
	ui->setupUi(this);

	connect(ui->select_dataset, SIGNAL(released()), this, SLOT(sl_selectDataPath()));
	connect(ui->start_processing, SIGNAL(released()), this, SLOT(sl_processData()));
}

ForceLookingDownGUI::~ForceLookingDownGUI()
{
	delete ui;
}

void ForceLookingDownGUI::sl_selectDataPath()
{
	QString data_folder = QFileDialog::getExistingDirectory(this, tr("Data path"),
		"./",
		QFileDialog::ShowDirsOnly
		| QFileDialog::DontResolveSymlinks);
	if (!data_folder.isEmpty())
	{
		if (QDir::separator() == "\\")
			data_folder.replace("/", "\\");

		ui->dataset_path->setText(data_folder);
	}
}

void ForceLookingDownGUI::sl_processData()
{

	QDirIterator dir_it(ui->dataset_path->text(), QStringList(), QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);

	QStringList sfm_data_to_process,obj_to_process;

	// search files
	while (dir_it.hasNext())
	{
		QDir current_dir = QDir(dir_it.next());

		QStringList obj_files = current_dir.entryList(QStringList() << "*.obj");
		QStringList sfm_data_files = current_dir.entryList(QStringList() << "sfm_data.bin");

		// we treat only in case of a unique model inside the folder
		if (obj_files.size() == 1 && sfm_data_files.size() == 1)
		{
			qDebug() << current_dir.filePath(obj_files[0]);

			sfm_data_to_process << current_dir.filePath(sfm_data_files[0]);
			obj_to_process << current_dir.filePath(obj_files[0]);
		}
	}

	// process files
	ui->overall_progress->setValue(0);
	for (int i=0; i<sfm_data_to_process.size(); i++)
	{
		processSfmObjFile(sfm_data_to_process[i], obj_to_process[i]);
		ui->overall_progress->setValue(int(100.0*double(i+1) / double(sfm_data_to_process.size())));
	}
	ui->overall_progress->setValue(100);

}

void ForceLookingDownGUI::processSfmObjFile(QString _sfm_data_file, QString _obj_file)
{

	// Load input SfM_Data scene
	SfM_Data* sfm_data = new SfM_Data;
	if (!Load(*sfm_data, _sfm_data_file.toStdString(), ESfM_Data(ALL)))
	{
		qDebug() << " cannot be read " << _sfm_data_file;
		return;
	}


	std::vector<Vec3> vec_sfm_fit_data, vec_sfm_lookdown_fit_data;
	std::vector<Mat3> vec_sfm_rot;

	// For front point computation
	Vec3 cam_front_point(0, 0, 10);

	for (const auto& view_it : sfm_data->GetViews())
	{
		if (!sfm_data->IsPoseAndIntrinsicDefined(view_it.second.get()))
			continue;

		const std::string view_filename =
			stlplus::create_filespec(sfm_data->s_root_path, view_it.second->s_Img_path);


		// get pose
		const openMVG::geometry::Pose3 pose(sfm_data->GetPoseOrDie(view_it.second.get()));

		vec_sfm_fit_data.push_back(pose.center());
		vec_sfm_fit_data.push_back(pose.rotation().transpose() * cam_front_point + pose.center()); // add forward looking point

		vec_sfm_lookdown_fit_data.push_back(pose.center());
		vec_sfm_lookdown_fit_data.push_back(pose.center() - cam_front_point); // add downlooking point
	}

	if (vec_sfm_fit_data.empty())
	{
		std::cerr << "No valid data found for the used views." << std::endl;
		return;
	}

	// Convert positions to the appropriate data container
	Mat X_sfm_fit_data = Eigen::Map<Mat>(vec_sfm_fit_data[0].data(), 3, vec_sfm_fit_data.size());
	Mat X_sfm_lookdown_fit_data = Eigen::Map<Mat>(vec_sfm_lookdown_fit_data[0].data(), 3, vec_sfm_lookdown_fit_data.size());

	openMVG::geometry::Similarity3 sim;

	Vec3 t;
	Mat3 R;
	double S=1.0;
	if (!FindRT(X_sfm_fit_data, X_sfm_lookdown_fit_data, &t, &R))
	{
		std::cerr << "Failed to comute the registration" << std::endl;
		return;
	}

	std::cout
		<< "Found transform:\n"
		<< " scale: " << S << "\n"
		<< " rotation:\n" << R << "\n"
		<< " translation: " << std::fixed << std::setprecision(9)
		<< t.transpose() << std::endl;

	// Encode the transformation as a 3D Similarity transformation matrix // S * R * X + t
	sim = openMVG::geometry::Similarity3(geometry::Pose3(R, -R.transpose() * t / S), S);

	// Apply the found transformation to the SfM Data Scene
	//-- transform ply
	//openMVG::sfm::ApplySimilarity(sim, *sfm_data);
	//Save(*sfm_data, "C:\\........\xxxxx.ply", ESfM_Data(ALL));
	//-- transform obj
	transformObjFile(_obj_file, t, R, S);
	
	delete sfm_data;
}

void ForceLookingDownGUI::transformObjFile(QString _obj_file, Vec3& t, Mat3& R, double& S)
{
	QFileInfo _obj_file_info(_obj_file);
	QString _out_obj_file = _obj_file_info.absoluteDir().absoluteFilePath(_obj_file_info.baseName() + "_rect.obj");

	ifstream in_obj_file;
	ofstream out_obj_file;
	in_obj_file.open(_obj_file.toStdString()); // open obj file

	int file_size = 1;
	int read_bytes = 0;

	string line;
	double v_x, v_y, v_z;

	// get file size
	in_obj_file.seekg(0, in_obj_file.end);
	file_size = in_obj_file.tellg();
	in_obj_file.seekg(0, in_obj_file.beg);

	// open out file
	out_obj_file.open(_out_obj_file.toStdString());

	// init progress bar
	ui->current_model_progress->setValue(0);

	if (in_obj_file.is_open())
	{
		while (getline(in_obj_file, line))
		{

			read_bytes += line.size();
			ui->current_model_progress->setValue(int(100.0*double(read_bytes) / double(file_size)));

			if (sscanf(line.c_str(), "v %lf %lf %lf", &v_x, &v_y, &v_z) == 3) {

				Vec3 vertex(v_x, v_y, v_z);
				Vec3 outvertex = S*R*vertex + t;
				out_obj_file << "v " << outvertex.x() << " " << outvertex.y() << " " << outvertex.z() << endl;

			}
			else if (sscanf(line.c_str(), "vn %lf %lf %lf", &v_x, &v_y, &v_z) == 3) {
				Vec3 vertexN(v_x, v_y, v_z);
				Vec3 outvertexN = R*vertexN;
				out_obj_file << "vn " << outvertexN.x() << " " << outvertexN.y() << " " << outvertexN.z() << endl;
			}
			else {
				out_obj_file << line << '\n'; // copy as is
			}

		}
		in_obj_file.close();
		out_obj_file.close();
	}
	else cout << "Unable to open file";

	ui->current_model_progress->setValue(100);
}
