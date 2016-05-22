#ifndef VIEW_H
#define VIEW_H

#include <QWidget>
#include <QPaintEvent>
#include <vector>
#include "State.h"
#include "Controller.h"

class Stage;

/**
 * @brief The View class
 * The class that controls what's displayed to the users
 */
class View : public QWidget {

    Q_OBJECT

public:
    /**
     * @brief View
     * Default constructor, initializes pointers to NULL
     */
    View();

    /**
     * View constructor
     */
    View(Stage* stage, Controller* controller, std::vector<State*> states);

    /**
     * View deconstructor
     */
    virtual ~View();

    /**
     * @brief initialize
     * function to initialize the View object
     */
    virtual void initialize() = 0;

public slots:

    /**
     * @brief update
     * Connected to signals from the different State objects
     * Is called to update the visual display whenever State changes
     * @param ID the id of the State that needs to be updated
     */
    virtual void update(int ID) = 0; //needs to be refactored to be able to update arbitrary number of states

protected:
    Stage* stage;
    Controller* controller;
    std::vector<State*> states;
};

#endif // VIEW_H
