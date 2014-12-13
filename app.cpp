#include "app.h"
#include <QDirIterator>

App::App():
    QMainWindow()
{
    p_detector = new PlateDetector();
    p_recognizer = new PlateRecognizer();

    work_mode = CollectMode;
}

App::~App()
{
    delete p_detector;
    delete p_recognizer;
}

int App::LoadImage(const string img_name)
{
    in_img = imread(img_name, 1);
    if (!in_img.data){ //cannot load image
        cerr << "Cannot load image!" << endl;
        return 0;
    }

    return 1;
}

int App::LoadDirectory(const string path_to_dir)
{
    //cout << "[Image Loader] Read images from " << path_to_dir << endl;
    //convert the directory path to QString
    QString q_path_to_dir = QString::fromStdString(path_to_dir);

    //iterate over the input directory to load all images
    QDirIterator dir_iter(QString(q_path_to_dir),
                          QDir::AllDirs|QDir::Files|QDir::NoSymLinks,
                          QDirIterator::Subdirectories
                          );

    while(dir_iter.hasNext())
    {
        dir_iter.next();

        //make sure it collects only image files
        if (dir_iter.fileInfo().completeSuffix().contains("jpg", Qt::CaseInsensitive) ||
                dir_iter.fileInfo().completeSuffix().contains("jpeg", Qt::CaseInsensitive) ||
                dir_iter.fileInfo().completeSuffix().contains("png", Qt::CaseInsensitive)
                ){
            // get the current file's RELATIVE file path
            QString current_file = dir_iter.fileInfo().filePath();

            //load image and file and add to the vector
            Mat cur_img = imread(current_file.toStdString(),1);
            in_img_vec.push_back(cur_img);
        }
    }
    return 1;

}

int App::Run()
{

#if (work_mode == CollectMode)

    std::string input_dir_path = "../../data/tmp/";
    std::string output_dir_path = "../../data/characters";

    //load all image from directory
    if (!LoadDirectory(input_dir_path))
        return 0;

    //for each image in the directory, perform plate detection
    vector<Mat> plates;
    int counter = 0;
    vector<Mat>::iterator iter = in_img_vec.begin();
    while(iter!=in_img_vec.end()){
        std::cout << "Process " << counter << " out of " << in_img_vec.size() << std::endl;
        plates.clear();
        p_detector->DetectPlate((*iter), plates);

        //for each plate image, extract and save character image
        for (int i=0; i<plates.size();i++){
            p_recognizer->SaveCharacterImg(plates[i],output_dir_path);
        }
        ++counter;
        ++iter;
    }

#elif (work_mode == TrainMode )

#elif (work_mode == RunMode )
    /* read 1 input image */
    std::string input_name = "../../data/P6070016.jpg";
    if (!LoadImage(input_name))
        return 0;


    //std::string content_plate;
    /* Recognize plate image */
    //for (int i=0; i<plates.size(); i++){
        //std::cout << "Processing plate " << (i+1) << " out of " << plates.size() << std::endl;
        //p_recognizer->RecognizePlate(plates[i], content_plate);
    //}
#endif

    return 1;
}
