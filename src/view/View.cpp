#include "View.h"
#include <QBrush>
#include <opencv2/imgproc.hpp>

View::View() {
    this->stage = NULL;
    this->controller = NULL;
}

View::View(Stage* stage, Controller* controller, std::vector<State *> states) {
    this->stage = stage;
    this->controller = controller;
    this->states = states;
}

View::~View() {

}
