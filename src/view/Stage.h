#ifndef STAGE_H
#define STAGE_H

#include <QWidget>
#include <QSize>
#include <QBoxLayout>
#include "Logger.h"
#include "Sub.h"
#include "SubFactory.h"

/**
 * @brief The Stage class
 * The Main window that's displayed when the program is running
 * Is a container for the different Views
 */
class Stage : public QWidget {

    Q_OBJECT

private:
    Logger* logger = new Logger("Stage");

    SubFactory* subFactory;
    Sub* sub;
    QBoxLayout* stageLayout;

public:
    /**
     * @brief Stage constructor
     * @param parent The parent widget if available, should be NULL
     * @param subFactory Factory object to create the various Sub configurations
     */
    explicit Stage(QWidget *parent = 0, SubFactory* subFactory = nullptr);

    /**
     * Stage destructor
     */
    ~Stage();

    /**
     * @brief Initializes the Stage with the default settings
     */
    void initialize();

    /**
     * @brief setViewContent Changes the current View to the new specified View
     * by deleting the previous View and stopping its Controller.
     * It's connections are also disconnected
     * @param type specifies the mode to switch to (@see SubType)
     */
    void setViewContent(std::string type);

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

signals:

public slots:
    /**
     * @brief switchToGUIView
     * Switches current view to display the cameras and command interface
     */
    void switchToGUIView(void);

    /**
     * @brief switchToSimulatorView
     * Switches current View to display the simulator
     */
    void switchToSimulatorView();

    /**
     * @brief switchToMenuView
     * Switches current View to display the Menu
     */
    void switchToMenuView(void);

    /**
     * Cleans the queue; forces the last task to finish, then kills the sub
     */
    void exit(void);


};

#endif // STAGE_H
