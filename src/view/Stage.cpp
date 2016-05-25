#include "Stage.h"

Stage::Stage(QWidget *parent, SubFactory* subFactory) : QWidget(parent) {
    this->subFactory = subFactory;
    subFactory->setStage(this);
    sub = nullptr;
    stageLayout = new QBoxLayout(QBoxLayout::LeftToRight, this);
    stageLayout->setSizeConstraint(QLayout::SetFixedSize);
}

void Stage::initialize() {
    setAttribute(Qt::WA_DeleteOnClose, true);
    this->setLayout(stageLayout);
    this->show();
}

void Stage::setViewContent(std::string type) {
    if (sub != nullptr && sub->getView() != nullptr) {
        logger->info("Deleting previous view");
        stageLayout->removeWidget(sub->getView());
        // Delete Sub instance
        // Delete and disconnects all associated QWidgets and their respective signals and slots
        delete sub;
    }
    if (type == "SIMULATOR"){        
        sub = subFactory->makeSub(type);
        sub->initialize();
        stageLayout->addWidget(sub->getView());
        this->show();
    }else{
        sub = subFactory->makeSub(type);
        sub->initialize();
        stageLayout->addWidget(sub->getView());
        this->show();
    }
    logger->info("New View initialized");
}

QSize Stage::sizeHint() const {
    return sub->getView()->sizeHint();
}

QSize Stage::minimumSizeHint() const {
    return sub->getView()->minimumSizeHint();
}

void Stage::switchToGUIView() {
    setViewContent("GUI");
}

void Stage::switchToSimulatorView() {
    setViewContent("SIMULATOR");
}

void Stage::switchToMenuView() {
    setViewContent("MENU");
}

void Stage::exit() {
    if (sub->getView() != nullptr) {
        sub->getView()->close();
        stageLayout->removeWidget(sub->getView());
        logger->debug("Removed View");
    }
    close();
}

Stage::~Stage() {
//    if (sub->getView() != nullptr) {
//        stageLayout->removeWidget(sub->getView());
//        logger->debug("Removed View");
//    }
    delete stageLayout;
    if (sub != nullptr) {
        delete sub;
        logger->debug("Deleted Sub");
    }
    delete subFactory;
    delete logger;
}
