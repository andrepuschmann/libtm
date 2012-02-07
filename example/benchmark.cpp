/*
 *
 *   This file is part of OSPECOR².
 *
 *   OSPECOR² is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   OSPECOR² is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with OSPECOR².  If not, see <http://www.gnu.org/licenses/>.
 *
 *   Copyright 2011, 2012 Andre Puschmann <andre.puschmann@tu-ilmenau.de>
 *
 */

#include <iostream>
#include "tm.hpp"

using namespace std;

int main()
{
    unsigned int counter = 0;
    timeMeasure test_rdtsc("Test RDTSC", RDTSC);
    timeMeasure test_gettime("Test gettime", GETTIME);
    
    test_rdtsc.benchmark();
    test_gettime.benchmark();

    boost::shared_ptr< timeMeasure > test1;
    test1.reset(new timeMeasure("Hello World1", GETTIME));
    
    for (int i = 0; i < 50000; i++)
    {
        test1->start();
        for (int k = 0; k < 50000; k++)
            counter++;
        test1->stop();
    }
    
    test1->printSummary(MSEC);

    return 0;
}
