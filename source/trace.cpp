


//#include <windows.h>
//
//#include "circular_buffer.h"
#include "trace.h"
//#include <cstdio>
//#include <iostream>
//#include <vector>
//#include <string>
#include "DBG_print.h"

void trace::put(double val_, double time_){
	//
	lowPass.put(val_, time_);	
	// raw trace has no time, this is stored in higher ->traces
	rawTrace.put(val_);        
	//
	//
	// For peakFinding use lowPass filtered Values
	double xm = lowPass.mean();
	fastTrace.put(xm, lowPass.centerTime());
	slowTrace.put(xm);
	//printf("trace::put val %.1f sum %.1f nval %d mean %.3f nEle %d Size %d\n", val_, lowPass.sum(), lowPass.NVAL(), lowPass.mean(), lowPass.NELE(), lowPass.size());
	//printf("trace::put val %.1f xm %.1f slow_min %.3f slow_max %.3f nEle %d Size %d\n", val_, xm, slowTrace.min_, slowTrace.max_, slowTrace.NELE(), slowTrace.size());
	double xi = val_ -lowPass.AVG();
	noise.put(xi * xi);
	//
	//first filter for peak height versus MAX/MIN*threash
	peakCut  = (slowTrace.MAX() - slowTrace.MIN()) * peakThresh;
	// AVG Noise = variance
	noiseCut = sqrt(noise.AVG()) * noiseThresh;
	cutOff   = slowTrace.MAX() - max(peakCut, noiseCut);

	// get square of xi-xm for center of fastTrace	
	//double xi = (fastTrace.center() - xm) / xm;
	//noise.put(xi*xi);
}

bool trace::findPeak()
{
	double dPeakRR  = 0.0;
	double dPeakAbs = 0.0;
	bool bPeak     = false;
	/*
	Analyze Frame that is currently in the center of runBufPeak to check it is a local maximum
	*/
	
	if (fastTrace.isMin()){
		//
		runMax = slowTrace.MAX();
		runMin = slowTrace.MIN(); 
		dPeakHeight = runMax - fastTrace.center();  //converts peak height to percentage
		//first filter for peak height versus MAX/MIN*threash
		peakCut  = (runMax - runMin) * peakThresh;
		noiseCut = noise.AVG() * noiseThresh;
		//		
		if ( dPeakHeight >  max(peakCut, noiseCut) ) 
		{
			//have a Peak (that is a Minimum)						
			dPeakTime = fastTrace.centerTime();
			dPeakRR   = (peakRel.NELE() > 0) ? dPeakTime - peakRel.getTime() : 0.0;
			//
			peakRel.put(dPeakHeight/runMax, dPeakTime);
			peakAbs.put(dPeakHeight+runMax, dPeakTime);
			peakRR.put(dPeakRR);
			bPeak=true;
			//printf("trace::put val %.1f sum %.1f nval %d mean %.3f nEle %d Size %d\n", val_, lowPass.sum(), lowPass.NVAL(), lowPass.mean(), lowPass.NELE(), lowPass.size());
		}
	}
	return bPeak;
}
