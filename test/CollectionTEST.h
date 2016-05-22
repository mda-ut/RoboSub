/*
 * CollectionTEST.h
 *
 *  Created on: Jun 3, 2015
 *      Author: ahsueh1996
 */

#ifndef TEST_COLLECTIONTEST_H_
#define TEST_COLLECTIONTEST_H_

#include "util/IDHasherTEST.h"

#include "util/filter/FilterManagerTEST.h"

#include "util/data/DataTEST.h"
#include "util/data/ImgDataTEST.h"
#include "util/data/FPGADataTEST.h"

#include "util/filter/RGBFilterTEST.h"
#include "../src/util/Logger.h"

class CollectionTEST {
public:
    int runDataAndFilterManagerCollection();
    int runFilterCollection();

private:
    Logger* logger = new Logger("CollectionTEST");
};

#endif /* TEST_COLLECTIONTEST_H_ */
