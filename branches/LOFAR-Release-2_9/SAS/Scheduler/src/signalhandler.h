#ifndef SIGNALHANDLER_H
#define SIGNALHANDLER_H

#include <QObject>
#include <QApplication>
#include <string>

// TOd prevent circular dependency on the controller declare instead of include controller
class Controller;

// SignalHandler QObject that forwards received actions trings to internals of the
// scheduler. It is expoced as interface on the ligscheduler via the function
// signalForward in libScheduler.h
class SignalHandler : public QObject
{
    Q_OBJECT

public:
    // Contructor, needs the internal objects QApplication and Controller
    explicit SignalHandler(QApplication *app, Controller *c);

    // Switch function emiting signals based on action string
    int signalForward(std::string action, std::string parameter);

signals:
    // TODO: We might need signals back:
    //   - gui opened
    //   - exit called
    //   - sas download complete
    //   - etc.

public slots:

private:
    Controller *itsController;
    QApplication *itsApplication;
};

#endif // SIGNALHANDLER_H
