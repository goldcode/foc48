#pragma once


#include "circular_buffer.h"
//#include <cstdio>
//#include <memory>
//#include <vector>
//#include <cmath>


class trace {
public:
	//fastTrace*fps, slowTrace*fps, allTrace*fps, lowPass, nPeak
	//circular_buffer(int size = 5, bool bStat = true, bool bTime = false);
	trace() : 
		fastTrace(15, true, true),           // Time required for peak
		slowTrace(151, true, false, true),   // Time required for peak
		lowPass(3, false, true),             // Time required for centerTime
		rawTrace(3600, false, true),         //include stats
		peakRel(11, true, true) ,
		peakAbs(11, true, true),
		noise(21,false, false)
	{
		// init code here
		slowTrace.name = L"SlowTrace";
	};
	
	virtual ~trace() {
		//destructor here
	};
	//
	void resize(int i, int j, int k, int l, int m){
		fastTrace.resize(i);
		noise.resize(j);		
		slowTrace.resize(j);
		rawTrace.resize(k);
		lowPass.resize(l);
		peakRel.resize(m);
		peakAbs.resize(m);
		peakRR.resize(m);
	};

	void reset() {
		fastTrace.reset();
		noise.reset();
		slowTrace.reset();
		rawTrace.reset();
		lowPass.reset();
		lowPass.initialize();
		peakRel.reset();
		peakAbs.reset();
		peakRR.reset();
	};
	
	void initialize() {
		fastTrace.initialize();
		noise.initialize();
		slowTrace.initialize();
		rawTrace.initialize();
		lowPass.initialize();
		peakRel.initialize();
		peakAbs.initialize();
		peakRR.initialize();
	};

	void put(double val_ , double time_);

	double   getBaseline() { return slowTrace.MAX(); };
	//
	double   getPeak()     { return peakRel.get(); }; //get the last Peak value
	long int getPeakN()    { return peakRel.NELE();};
	double   getPeakAvg()  { return peakRel.AVG(); };
	double   getPeakDev()  { return peakRel.DEV(); }; 
	double   getPeakSkew() { return peakRel.skewness(); };
	double   getPeakKurt() { return peakRel.kurtosis(); };
	double   getPeakMin()  { return peakRel.MIN(); }; 
	double   getPeakMax()  { return peakRel.MAX(); };
	
	double   getRR()       { return peakRR.get(); };  //get the last RR value
	double   getRRAvg()    { return peakRR.AVG(); };
	double   getRRDev()    { return peakRR.DEV(); };
	double   getRRSkew()   { return peakRR.skewness(); };
	double   getRRKurt()   { return peakRR.kurtosis(); };
	double   getRRMin()    { return peakRR.MIN(); };
	double   getRRMax()    { return peakRR.MAX(); };

	double*  getCutOffPtr()   { return &cutOff; };

	double   getDistance(int iIn){ return rawTrace.getAt(iIn); };
	double   getNoiseAvg() { return noise.mean(); };
	double   getSNR()      { return log10(getPeakAvg()/noise.AVG()); };

	double   getTraceMin()  { return rawTrace.MIN(); };
	double   getTraceMax()  { return rawTrace.MAX(); };
	double   getTraceAvg()  { return rawTrace.AVG(); };
	double   getTraceDev()  { return rawTrace.DEV(); };
	double   getTraceSkew() { return rawTrace.skewness(); };
	double   getTraceKurt() { return rawTrace.kurtosis(); };

	double   slowTraceAVG() { return slowTrace.AVG(); };

	bool findPeak();
	int id = 0;    //for identification
	//	
	circular_buffer  fastTrace;      //holds low_pass filtered distance and time
	circular_buffer  slowTrace;      //holds low_pass filtered distance
	circular_buffer  noise;
	circular_buffer  lowPass;        //holds raw data and time
	circular_buffer  rawTrace;       //holds raw data
	//
	circular_buffer  peakRel;        //holds Peak Height as % of Baseline and Frame Number
	circular_buffer  peakAbs;        //holds absolute Peak Height and time e.g. for Plotting
	circular_buffer  peakRR;         //holds Peak-Peak distance	
	//
	double peakCut = 0.0;
	double noiseCut = 0.0;
	double cutOff = 0.0;      // Absolute cutOff, slowTrace.Max - max(peakThresh,noiseThresh)
	double peakThresh = 0.5;
	double noiseThresh = 6.0;
private: 
	double dPeakHeight = 0.0;
	double dPeakTime   = 0.0;
	double runMax      = 0.0;
	double runMin      = 0.0;
	
	
};



