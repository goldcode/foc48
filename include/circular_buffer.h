
#pragma once

#include <stdlib.h>	// NULL
#include <math.h>	// HUGE_VAL
#include <atlstr.h>  // for CString

//#include "stdafx.h"
//#include <cstdio>
//#include <memory>
//#include <vector>
//#include <cmath>
//#include <math.h> 
//
//#include <atlstr.h>
//#include <cstdio>
//#include <cstring>

class circular_buffer {
public:

	circular_buffer(int size = 5, bool bMinMax = true, bool bTime = false, bool bStats = false) {
		resize(size);
		bMinMax_ = bMinMax;
		bTime_ = bTime;
		bStats_ = bStats;
	};
	
	virtual ~circular_buffer() { free(); };

	void free(){ 
		if (buf_ != NULL)  delete[] buf_; 
		if (time_ != NULL) delete[] time_;		
	}

	void reset(){		
		nele = 0;
		indx = -1;
		sum_ = 0.0;
		min_ = HUGE_VAL;
		max_ = -HUGE_VAL;
		minIndex = -1;
		maxIndex = -1;
	}
	
	void resize(int newSize) {		
		free();
		// Oddify buffer size
		size_ = (newSize % 2) == 0 ? newSize + 1 : newSize; 
		buf_ = new double[size_];
		time_ = (bTime_)  ? new double[size_] : NULL;
		//
		reset();
	}

	void initializeArray(double* pArr, double dIn = 0.0) {
		for (indx = 0; indx < size_; ++indx)
			pArr[indx] = dIn;
	}

	void initialize(double dIn = 0.0) {		
		initializeArray(buf_, 0.0);
		if (bTime_) initializeArray(time_,0.0);		
		reset();
	}
	
	//void     initialize();

	void     put  (double item_in, double time_in=0.0);
	//void     put_t(double dIn) { put(dIn); };
	//
	double   get()            { return   buf_[indx]; };
	double   getLast()        { return   buf_[indx]; };
	double   getFirst()       { return   buf_[firstEle()]; };

	double   getAt(int iIn)   { return   buf_[ (nele-iIn) % size_]; };
	int      firstEle()       { return (nele > size_) ? nele-size_ : 0; };	
	bool     isFilled() { return (nele > size_); };
	double   getEle(int iIn)  { return buf_[iIn % size_] ;};
	double * getPtr()         { return &(buf_[indx]); };
	bool     isMin()          { return centerIndex() == minIndex; };
	bool     isMax()          { return centerIndex() == maxIndex; };
	int      index()          { return indx; };
	double   center()	      { return buf_[centerIndex()]; };
	int      centerIndex()    { return (indx + (size_ + 1) / 2) % size_; };
	int      centerNum()      { return (nele - (size_ + 1) / 2) ; };
	double   centerTime()     { return time_[centerIndex()]; };
	double*  centerTimePtr()  { return &(time_[centerIndex()]); };
	double*  centerPtr()      { return &(buf_[centerIndex()]); };
	double   getTime()        { return time_[indx]; };
	double*  getTimePtr()     { return &(time_[indx]); };

	double read_back(int i)	{ return buf_[num2indx(nele - i)]; };

	int     size(void)        { return size_; };
	bool    bMinMax(void)     { return bMinMax_;	}
	bool    bStats(void)      { return bStats_; }
	double  AVG(void)         { return (double)sum_ / NVAL(); };   //alias to mean
	double  DEV(void)         { return stdev(); };				   //alias to stdev
	double  MIN(void)         { return min_; };
	double* minPtr(void)      { return &min_; };
	double	MAX(void)         { return max_; };
	double* maxPtr(void)      { return &max_; };
	double	range(void)       { return max_ - min_; };

	int	    NELE(void)        { return nele; };                    // all elements ever entered
	int	    NVAL(void)        { return (nele > size_) ? size_ : nele; }; // number of elements in buffer
	double  sum(void)         { return sum_; };
	double  mean(void)        { return (double)sum_ / NVAL(); };
	int     num2indx(int num) { return (num > size_) ? num % size_ : num; }; 
	double  stdev(void);
	double  item_out;
	void    find_max(void);
	void    find_min(void);
	double  rms() { double xi = center() - mean();  return xi*xi; };
	//double rmsd() { return sqrt(sum_) / size_; };
	double skewness();
	double kurtosis();
	//	
	int     size_ = 0;      // number of elements if buffer

	double * buf_ = NULL;
	double * time_= NULL;
	
	int     nele = 0;
	double	min_ =  HUGE_VAL;
	double	max_ = -HUGE_VAL;
	
	CString name =  "cbuf";
	int      num =  0;

private:
	int i  = 0;
	
	//int size_    = 0;
	double  sum_  = 0;
	int     indx  = -1;   
	int		maxIndex = -1; //index of the maximum value
	int		minIndex = -1; //index of the minimum value	
	// index of last element
	bool bMinMax_ = true;
	bool bStats_  = true;
	bool bTime_   = false;
};

