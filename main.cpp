#include "app.h"
#include <QApplication>

//main routine
int main(int argc, char* argv[])
{
    QApplication MainApp(argc, argv);

    App m_app;

    return m_app.Run();
}
