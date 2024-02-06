#include "circular_buffer.h"
//#include <cstdio>
//#include <iostream>
//#include <string>

#include "DBG_print.h"

void circular_buffer::put(double item_in, double time_in)
{
	indx = (indx + 1 == size_) ? 0 : indx + 1;
	nele = nele + 1;
	//	
	item_out = isFilled() ? buf_[indx]: 0.0;
	//
	buf_[indx] = item_in;
	// fix statistics
	sum_ = sum_ + item_in - item_out;
	//
	//
	if (bMinMax_){
		if (item_in > max_) {
			max_ = item_in;
			maxIndex = indx;
		} else if (item_out == max_)	{
			find_max();
		}
		//
		if (item_in < min_){
			min_ = item_in;
			minIndex = indx;
		} else if (item_out == min_)
		{
			find_min();
		}
	}

	if (bTime_)
		time_[indx] = time_in;	
}


void circular_buffer::find_max(void)
{
	max_ = buf_[(maxIndex = 0)]; // max_ = buf_[0]; AAA, when max is buf_[0], maxIndex will be not defined
	for (int i = 1; i < size_; i++) // for (int i = 0; i < size_; i++), start from 1, because i=0,  (buf_[i]> max) will not run anyway.
	{
		if (buf_[i]> max_)
		{
			max_ = buf_[i];
			maxIndex = i;
		}
	}
}


void circular_buffer::find_min(void)
{
	//min_ = buf_[(minIndex = 0)];
	minIndex = 0; 
	min_ = buf_[minIndex];
	for (int i = 1; i < size_; i++)
	{
		if (buf_[i]< min_)
		{
			min_ = buf_[i];
			minIndex = i;
		}
	}
}


double circular_buffer::stdev(void)
{
	// sum of squares
	double ss2 = 0.0;
	double avg = mean();
	int nVal = NVAL();
	//

	for (int i = 0; i < NVAL(); i++)
	{
		ss2 += (buf_[i] - mean()) * (buf_[i] - mean());
	}
	//std::cout << "stdev mean " << avg << " NVAL " << nVal << " ss2 " << ss2 << " stdev " << sqrt(ss2 / nVal) << std::endl;
	return sqrt(ss2 / NVAL());
}

double circular_buffer::skewness(void)
{
	/*
	           1           x_i-x_avg
	SKEWNESS= ---  Sum (  ----------- )^3
	           N                s
	*/	
	double avg = AVG();
	double dev = DEV(); 
	double sum3 = 0.0;
	//
	for (int i = 0; i < NVAL(); i++)
		sum3 = sum3 + pow( (buf_[i] - avg)/dev, 3);
	
	return sum3 / NVAL();
}

double circular_buffer::kurtosis(void)
{
	/*
				1            x_i-x_avg   
	KURTOSIS=  ---  Sum (  ------------ )^4
	            N	            s
	*/
	double avg = AVG();
	double dev = 0.0;
	double sum4 = 0.0;
	//
	for (int i = 0; i < NVAL(); i++)
		sum4 = sum4 + pow((buf_[i] - avg) / dev, 4);
	//
	return sum4 / NVAL();
}
