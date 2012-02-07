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


#ifndef TM_HPP_
#define TM_HPP_

#include <iostream>
#include <string>
#include <vector>
#include <time.h>
#include <sys/time.h>
#include <stdint.h> // uint64_t
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/thread.hpp>


#define NSEC_PER_SEC    1000000000
#define REMOVE_MAX_MIN  0

enum tmmode {
    GETTIME = 0,
    RDTSC
};
typedef tmmode tmmode_t;

enum tmunit {
    NSEC = 0,
    USEC,
    MSEC
};
typedef tmunit tmunit_t;

static double m_ticksPerNsec;

class timeMeasure
{

public:
    //! constructor
    timeMeasure(std::string description, tmmode_t mode);
    //! destructor
    ~timeMeasure() { /*printSummary(MSEC);*/ };

    void start(void);
    void stop(void);
    void reset(void);

    double getMinNsec();
    double getMeanNsec();
    double getMaxNsec();
    double getSdNsec();

    double getMinUsec();
    double getMeanUsec();
    double getMaxUsec();
    double getSdUsec();

    double getMinMsec();
    double getMeanMsec();
    double getMaxMsec();
    double getSdMsec();

    void benchmark(void);
    void printSummary(tmunit_t unit);
    bool calculateStats(void);

private:
    std::string m_description;
    tmmode_t m_mode;

    // statistical stuff
    uint64_t m_minValue, m_maxValue;
    double m_meanValue, m_sdValue;

    std::vector<struct timespec> m_sampleVector;

    // timing stuff
    struct timespec m_startTime, m_stopTime, m_diffTime;
    uint64_t m_startTick, m_stopTick, m_diffTick;

    // private member functions
    static uint64_t readRdtsc(void);
    static void calibrateTicks(void);
    
    // time conversion helpers
    void timespecNorm(struct timespec *ts);
    static struct timespec timespecDiff(struct timespec start, struct timespec end);
    static uint64_t timespecToUsec(struct timespec time);
    static uint64_t timespecToNsec(struct timespec time);
    double nsecToUsec(uint64_t time);
    double nsecToMsec(uint64_t time);
    struct timespec tickToTimespec(uint64_t tick);
};


#endif /* TM_HPP_ */
