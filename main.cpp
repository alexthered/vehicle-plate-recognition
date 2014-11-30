#include "app.h"
#include <QApplication>


//main routine
int main(int argc, char* argv[])
{
    QApplication MainApp(argc, argv);

    //get the application
    App m_app;
    m_app.Run();

    return MainApp.exec();
}
