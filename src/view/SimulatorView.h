#ifndef SIMULATORVIEW_H
#define SIMULATORVIEW_H

#include <QWindow>

#include "Logger.h"
#include "View.h"

class SimulatorView : public View {
public:
    SimulatorView(Stage* stage, Controller* controller, std::vector<State*> states);
    ~SimulatorView();

    virtual void initialize();
    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

    QWindow* getWindow();

public slots:
    virtual void update(int id);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    Logger* logger = new Logger("SimulatorView");

    QWidget* container;
    QWindow* window;
    QSurfaceFormat* format;

public slots:
    void exit();

};

#endif // SIMULATORVIEW_H
