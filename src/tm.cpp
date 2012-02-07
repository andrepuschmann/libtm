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

#include "tm.hpp"

/*
 * TODO:
 * - proper error handling
 * - check whether RDTSC is available on platform
 * -
 */

timeMeasure::timeMeasure(std::string description, tmmode_t mode = GETTIME)
{
    m_description = description;
    m_mode = mode;
        
    if (m_mode == RDTSC)
    {
        // bind to last cpu (reading tsc from different cores might screw results
        cpu_set_t mask;
        CPU_ZERO(&mask);
        int max_cpus = sysconf(_SC_NPROCESSORS_CONF);
        CPU_SET(max_cpus - 1, &mask);
        std::cout << boost::format("Bind thread to CPU #%d\n") % max_cpus;
        if(sched_setaffinity(0, sizeof(mask), &mask) == -1)
            std::cout << boost::format("Could not set CPU affinity\n");
        
        // calculate ns per tick
        calibrateTicks();
        std::cout << boost::format("Ticks per Nsec: %Ld\n") % m_ticksPerNsec;

        // ...
    }
};

void timeMeasure::start(void)
{
    switch (m_mode)
    {
        case GETTIME:
            clock_gettime(CLOCK_MONOTONIC, &m_startTime);
            break;
            
        case RDTSC:
            m_startTick = readRdtsc();
            break;
    }
}

void timeMeasure::stop(void)
{
    switch (m_mode)
    {
        case GETTIME:   
            clock_gettime(CLOCK_MONOTONIC, &m_stopTime);
            m_diffTime = timespecDiff(m_startTime, m_stopTime);
            break;
            
        case RDTSC:
            m_stopTick = readRdtsc();
            m_diffTick = m_stopTick - m_startTick;
            m_diffTime = tickToTimespec(m_diffTick);
            break;
    }

    timespecNorm(&m_diffTime);
    m_sampleVector.push_back(m_diffTime);
}

void timeMeasure::reset(void)
{
    m_sampleVector.clear();
}


/*
 * getter functions for nanosecond time unit
 */
double timeMeasure::getMinNsec()
{
    return m_minValue;
}

double timeMeasure::getMeanNsec()
{
    return m_meanValue;
}

double timeMeasure::getSdNsec()
{
    return m_sdValue;
}

double timeMeasure::getMaxNsec()
{
    return m_maxValue;
}


/*
 * getter functions for microsecond time unit
 */
double timeMeasure::getMinUsec()
{
    return nsecToUsec(m_minValue);
}

double timeMeasure::getMeanUsec()
{
    return nsecToUsec(m_meanValue);
}


double timeMeasure::getSdUsec()
{
    return nsecToUsec(m_sdValue);
}

double timeMeasure::getMaxUsec()
{
    return nsecToUsec(m_maxValue);
}


/*
 * getter functions for millisecond time unit
 */
double timeMeasure::getMinMsec()
{
    return nsecToMsec(m_minValue);
}

double timeMeasure::getMeanMsec()
{
    return nsecToMsec(m_meanValue);
}


double timeMeasure::getSdMsec()
{
    return nsecToMsec(m_sdValue);
}

double timeMeasure::getMaxMsec()
{
    return nsecToMsec(m_maxValue);
}


bool timeMeasure::calculateStats(void)
{
    double sum;
    
    // check whether we have to recalculate the stats
    if (m_sampleVector.size() == 0)
    {
        std::cout << "No samples for '" << m_description << "' collected." << std::endl;
        return false;
    }

    m_minValue = 9999999999;
    m_maxValue = 0; 
    sum = 0;

#if REMOVE_MAX_MIN // FIXME: quick'n'dirty implemenatation
    uint64_t minValue = 9999999999, maxValue = 0;
    std::vector<struct timespec>::iterator min_val_pos;
    std::vector<struct timespec>::iterator max_val_pos;

    for(std::vector<struct timespec>::iterator it = m_sampleVector.begin(); it != m_sampleVector.end(); it++)
    {
        if (timespecToNsec(*it) < minValue)
        {
            minValue = timespecToNsec(*it);
            min_val_pos = it;
        }
        if (timespecToNsec(*it) > maxValue)
        {
            maxValue = timespecToNsec(*it);
            max_val_pos = it;
        }
    }
    m_sampleVector.erase(min_val_pos);
    m_sampleVector.erase(max_val_pos);
#endif

    sum = 0;
    BOOST_FOREACH(struct timespec sample, m_sampleVector)
    {
        // don't include max and min
        sum += timespecToNsec(sample);
        m_minValue = std::min(m_minValue, timespecToNsec(sample));
        m_maxValue = std::max(m_maxValue, timespecToNsec(sample));
    }
    m_meanValue = sum / m_sampleVector.size();
    
    // calculate standard deviation
    sum = 0;
    BOOST_FOREACH(struct timespec sample, m_sampleVector)
    {
        // calculate difference from mean, take the power of two and sum it up  
        sum += pow(timespecToNsec(sample) - round(m_meanValue), 2);
    }

    // compute average of sum and take square root
    m_sdValue = sqrt(std::fabs(sum) / (m_sampleVector.size() - 1));
    
    return true;
}

void timeMeasure::printSummary(tmunit_t unit = MSEC)
{
    if (!calculateStats())
        return;

    std::cout << boost::format("------") << std::endl;
    std::cout << boost::format("Desc     : %s\n") % m_description;
    std::cout << boost::format("#Samples : %s\n") % m_sampleVector.size();

    switch (unit)
    {
        case NSEC:
            std::cout << boost::format("Min      : %.9ld nsec\n") % getMinNsec();
            std::cout << boost::format("Mean     : %.9ld nsec\n") % getMeanNsec();
            std::cout << boost::format("Max      : %.9ld nsec\n") % getMaxNsec();
            std::cout << boost::format("StdDev   : %.9ld nsec\n") % getSdNsec();
            break;

        case USEC:
            std::cout << boost::format("Min      : %.2f usec\n") % getMinUsec();
            std::cout << boost::format("Mean     : %.2f usec\n") % getMeanUsec();
            std::cout << boost::format("Max      : %.2f usec\n") % getMaxUsec();
            std::cout << boost::format("StdDev   : %.2f usec\n") % getSdUsec();
            break;

        case MSEC:
            std::cout << boost::format("Min      : %.2f msec\n") % getMinMsec();
            std::cout << boost::format("Mean     : %.2f msec\n") % getMeanMsec();
            std::cout << boost::format("Max      : %.2f msec\n") % getMaxMsec();
            std::cout << boost::format("StdDev   : %.2f msec\n") % getSdMsec();
            break;

    }
    std::cout << boost::format("------\n");
}


/* assembly code to read the TSC */
uint64_t timeMeasure::readRdtsc(void)
{
#ifdef __i386
    uint64_t val;
    __asm__ volatile ("rdtsc" : "=A" (val));
    return val;
#elif defined __amd64
    unsigned int hi, lo;
    __asm__ volatile("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
#else
    return 0;
#endif
}


void timeMeasure::calibrateTicks(void)
{
    struct timespec startTime, stopTime, diffTime;
    uint64_t startTick = 0, stopTick = 0;
  
    clock_gettime(CLOCK_MONOTONIC, &startTime);
    startTick = readRdtsc();
    for (uint64_t i = 0; i < 1000000; i++); // must be CPU intensive
    stopTick = readRdtsc();
    clock_gettime(CLOCK_MONOTONIC, &stopTime);
    diffTime = timespecDiff(startTime, stopTime);
    m_ticksPerNsec = (double)(stopTick - startTick)/(double)timespecToNsec(diffTime);
}



/* the struct timespec consists of nanoseconds
 * and seconds. if the nanoseconds are getting
 * bigger than 1000000000 (= 1 second) the
 * variable containing seconds has to be
 * incremented and the nanoseconds decremented
 * by 1000000000.
 */
void timeMeasure::timespecNorm(struct timespec *ts)
{
   while (ts->tv_nsec >= NSEC_PER_SEC) {
      ts->tv_nsec -= NSEC_PER_SEC;
      ts->tv_sec++;
   }
}


struct timespec timeMeasure::timespecDiff(struct timespec start, struct timespec end)
{
    struct timespec temp;
    if ((end.tv_nsec - start.tv_nsec) < 0)
    {
        temp.tv_sec = end.tv_sec - start.tv_sec - 1;
        temp.tv_nsec = NSEC_PER_SEC + end.tv_nsec - start.tv_nsec;
    }
    else
    {
        temp.tv_sec = end.tv_sec - start.tv_sec;
        temp.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
    return temp;
}

uint64_t timeMeasure::timespecToNsec(struct timespec time)
{
    return (time.tv_sec * NSEC_PER_SEC + time.tv_nsec);
}


uint64_t timeMeasure::timespecToUsec(struct timespec time)
{
    return (timespecToNsec(time) / 1000);
}

double timeMeasure::nsecToUsec(uint64_t time)
{
    return (time / static_cast<double>(1000));
}

double timeMeasure::nsecToMsec(uint64_t time)
{
    return (time / static_cast<double>(1000000));
}

struct timespec timeMeasure::tickToTimespec(uint64_t tick)
{
    struct timespec temp;
    
    temp.tv_sec = 0;
    temp.tv_nsec = tick / m_ticksPerNsec;

    return temp;
}

void timeMeasure::benchmark(void)
{
    m_sampleVector.clear();
    for (int i = 0; i < 1111; i++)
    {
        timeMeasure::start();
        boost::this_thread::sleep(boost::posix_time::milliseconds(1));
        timeMeasure::stop();
    }
    
    //printSummary(NSEC);
    //printSummary(USEC);
    printSummary(MSEC);

#if 0
    struct timespec start, end, diff;
    uint64_t tick1, tick2;

    tick1 = readRdtsc();
    tick2 = readRdtsc();
    std::cout << boost::format("RDTSC (in tics): Tick1: %Ld, Tick2: %Ld, Diff: %Ld\n") \
                                % tick1 % tick2 % (tick2 - tick1);

    clock_gettime(CLOCK_MONOTONIC, &start);
    clock_gettime(CLOCK_MONOTONIC, &end);
    diff = timespecDiff(start, end);
    std::cout << boost::format("clock_gettime() (in nsec): Tick1: %ld, Tick2: %ld, Diff: %ld\n") \
                                % start.tv_nsec % end.tv_nsec % diff.tv_nsec;
#endif
}
