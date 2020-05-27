#ifndef DVFS_SCHEDULE_H
#define DVFS_SCHEDULE_H

#include <DVFS/misc.h>
#include <string>
using namespace std;

inline int schedule(float &slack,  float Uncertainty, int  current_configuration)
{
	if(slack < 0)
		current_configuration += 100.0 /Uncertainty;
	else
	{
		if(Uncertainty>3)
		{
			current_configuration -= 10.0/Uncertainty;
		}
		else
		{
			current_configuration += 100.0/Uncertainty;
		}
	}
	return current_configuration;
}


inline int schedule(float &slack, float expected_deadline, float measured_exec, float Uncertainty, int  current_configuration)
{
	slack += measured_exec - expected_deadline;
	
	return schedule(slack, Uncertainty, current_configuration);
}

inline int schedule(float &slack, float expected_deadline, float previous_expected_deadline, float measured_exec, float Uncertainty, int prev_current_configuration,int current_configuration, std::map<int,hist_ent> &prev_history, std::map<string,int> &rev_prev_history, std::map<int,hist_ent> &history, std::map<string,int> &rev_history)
{
	//std::cout << "Scheduling." << std::endl;
	slack += measured_exec - previous_expected_deadline;
    
//	std::cout << "Loading previous run into history." << std::endl;

	if(prev_history.find(current_configuration) != prev_history.end())
	{
	}
	else
	{
		hist_ent tmp_ent;
		tmp_ent.speedup = previous_expected_deadline/measured_exec;
		prev_history[prev_current_configuration] = tmp_ent;
		rev_prev_history[to_string(tmp_ent.speedup)] = prev_current_configuration;
	}
	
	//std::cout << "Looking in current history." << std::endl;
/* to-do: use boost multi index */	
	float needed_speedup = expected_deadline / (expected_deadline+slack);
	if(needed_speedup<0)
	{
		needed_speedup = 7;
	}
	//cout << "++++ needed_speedup: " << needed_speedup << endl;
	if(rev_history.find(to_string(needed_speedup)) != rev_history.end())
	{ 
		current_configuration = rev_history[to_string(needed_speedup)];
	}else{
		current_configuration = schedule(slack, Uncertainty, current_configuration);
	}
	return current_configuration;
}

#endif
