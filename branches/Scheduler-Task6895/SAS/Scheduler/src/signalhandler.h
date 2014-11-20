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
    explicit SignalHandler(QApplication &app, Controller &c);

    // Switch function emiting signals based on action string
    int signalForward(std::string action, std::string parameter);

    // I do not like the idea of all these accessor functions.
    // THis signal handler is the start of the message interface.
    // Wait for the second test to see what the best generic implementation
    // would be. A map?
    bool getStatusSASDialogFeedbackResult();


signals:
    void mainWindowClose();
    void downloadSASSchedule();
    void closeSASScheduleDownloadDialog();
    void doNotSaveSchedule();
    void checkSASStatus();
    void closeCheckSASStatusDialog();

    // TODO: We might need signals back:
    //   - gui opened
    //   - exit called
    //   - sas download complete
    //   - etc.

public slots:
    void statusSASDialogFeedback(bool result);

private:
    // Connects all the available signals to the main application
    void connectSignals(void);

    Controller *itsController;
    QApplication *itsApplication;

    // A number of variable needed to get feedback via the signals/slot mechanis,
    bool statusSASDialogFeedbackResult;

};

#endif // SIGNALHANDLER_H
