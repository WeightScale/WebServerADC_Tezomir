/*
 * Filter.h
 *
 * Created: 24.12.2017 8:06:47
 *  Author: Kostya
 */ 


#ifndef FILTER_H_
#define FILTER_H_

#pragma once

/*
* Implements a simple linear recursive exponential filter.
* See: http://www.statistics.com/glossary&term_id=756 */
template<class T> class ExponentialFilter{
	
	// Weight for new values, as a percentage ([0..100])
	T m_WeightNew;

	// Current filtered value.
	T m_Current;

	public:
	ExponentialFilter() : m_WeightNew(100), m_Current(0){}
	ExponentialFilter(T WeightNew, T Initial) : m_WeightNew(WeightNew), m_Current(Initial){}

	void Filter(T New){
		m_Current = (m_WeightNew * New + (100 - m_WeightNew) * m_Current) / 100;
	}

	void SetFilterWeight(T NewWeight){
		m_WeightNew = NewWeight;
	}

	T GetFilterWeight() const { return m_WeightNew; }

	T Current() const { return m_Current; }

	void SetCurrent(T NewValue){
		m_Current = NewValue;
	}
};

// Specialization for floating point math.
template<> class ExponentialFilter<float>{
	float m_fWeightNew;
	float m_fCurrent;

	public:
	ExponentialFilter() : m_fWeightNew(1.0f), m_fCurrent(0.0f){}
	ExponentialFilter(float fWeightNew, float fInitial)	: m_fWeightNew(fWeightNew/100.0), m_fCurrent(fInitial){}


	void Filter(float fNew){
		m_fCurrent = m_fWeightNew * fNew + (1.0 - m_fWeightNew) * m_fCurrent;
	}

	void SetFilterWeight(float NewWeight){
		m_fWeightNew = NewWeight/100.0;
	}

	float GetFilterWeight() const { return m_fWeightNew*100.0; }

	float Current() const { return m_fCurrent; }

	void SetCurrent(float fNewValue){
		m_fCurrent = fNewValue;
	}
};




#endif /* FILTER_H_ */