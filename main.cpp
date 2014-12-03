#include "app.h"
#include <QApplication>


//main routine
int main(int argc, char* argv[])
{
    QApplication MainApp(argc, argv);

    //get the application
    App m_app;

#if PLOT_DISPLAYING
    m_app.Run();
    return MainApp.exec();
#else
    return m_app.Run();
#endif
}
