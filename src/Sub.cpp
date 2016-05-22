/*
 * Sub.cpp
 *
 *  Created on: Jan 8, 2015
 *      Author: mda
 */

#include "Sub.h"
#include <QCoreApplication>

volatile int signal_quit = 0;

Sub::Sub(std::vector<Model *> models_, View *view_, Controller *controller_) {
    models = models_;
    view = view_;
    controller = controller_;
}

Sub::~Sub() {
    for (auto& model : models) {
        delete model;
    }
    delete view;
    delete controller;
}

void Sub::initialize() {
    controller->initialize();
    view->initialize();
    for (auto& model : models) {
        model->initialize();
    }
}

View *Sub::getView() {
    return view;
}

