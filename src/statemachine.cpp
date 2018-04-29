#include "statemachine.h"



StateMachine::StateMachine()
{
	current_state = LK;

	// Successor states of Lane Keep:
	successor_states[LK].push_back(LK);
	successor_states[LK].push_back(LCL);
	successor_states[LK].push_back(LCR);

	// Successor states of Lane Change Left:
	successor_states[LCL].push_back(LCL);
	successor_states[LCL].push_back(LK);

	// Successor states of Lane Change Right:
	successor_states[LCR].push_back(LCR);
	successor_states[LCR].push_back(LK);

	weights.push_back(1.0);
	weights.push_back(1.0);

	current_lane = 1;
	intended_lane = 1;

}

StateMachine::~StateMachine() {
}

void successor_states() {
}




// Costs due to near-collision with other vehicles
double StateMachine::cost_function_0(const Trajectory &trajectory, const pose egoPose, const targetList_t list)
{
	double cost = 0;
	/*
	bool collision_imminent = false;

	// Check if there is a vehicle on the left lane:
	// Make the cost very high if the lane is occupied!

	for (int i=0; i<list.size(); i++)
	{
		double target_d = list[i][6];
		double target_s = list[i][5];
		double target_vx = list[i][3];
		double target_vy = list[i][4];
		double target_speed = sqrt(target_vx*target_vx+target_vy*target_vy);

		for (int j=0; j<50; j++)
		{
			double target_pos = target_speed*j*0.02;

			/*double dist = distance(target_pos-trajectory.xy(j*0.02));

			if (distance(egoPose-target_pos)<5.)
			{
				cost = 1/dist;
			}
		}
	}*/

	/*if (collision_imminent=true)
		cost = UINT_MAX;
	else
		cost = 0;*/

	return cost;
}

// Costs due to trajectory dynamics
double StateMachine::cost_function_1(const Trajectory &trajectory)
{
	double cost = 0;
	/*unsigned int end_of_path = trajectory[0].size()-1;

	double delta_d = abs(trajectory[1][end_of_path] - 4*current_lane+2);
	// The faster the trajectory is, the better!
	cost = exp( -trajectory[0][end_of_path] / delta_d );*/
	//std::cout << "Cost: " << cost << std::endl;
	return cost;
}



trajectory_t StateMachine::evaluate_behavior(pose egoPose, targetList_t vehicle_list, int rest)
{
	state best_next_state;
	remaining_points = rest;
    unsigned int min_cost = UINT_MAX;
    
    //std::map<state, double> costs;
    
    //std::cout << "Current state: " << current_state << std::endl;
    //for (auto iter : vehicle_list)
    //	cout << iter[0] << endl;

	// Frenet coordinates of ego-vehicle:
    std::vector<double> egoFrenet = maptool.getFrenet(egoPose.pos_x, egoPose.pos_y, egoPose.angle);
    egoPose.s = egoFrenet[0];
    egoPose.d = egoFrenet[1];

    // Allocating space for Trajectory object:
    Trajectory traj = Trajectory(LK, egoPose);
    Trajectory* best_next_trajectory;

    // Checks whether the ego vehicle has arrived at the intended lane.
    if (abs(egoPose.s-4*intended_lane+2)<0.2) {
    	current_lane = intended_lane;
    	current_state = LK;
    }

    for(auto state_iter : successor_states[current_state])
    {
		// Excluding the cases where the ego vehicle is on the left lane or the right land and considers lane change 
		// to nonexisting lanes.
	    if (!(state_iter==LCL && current_lane==0) && !(state_iter==LCL && current_lane==2))
	    {
	    	// One more for-loop here!
	    	for (int j=0; j<1; j++)
	    	{
			    double cost_for_state = 0;

			    // Generate new trajectory for state state_iter
				traj = Trajectory(state_iter, egoPose);

				// Add cost function for obstacles and road departure 
				cost_for_state += cost_function_0(traj, egoPose, vehicle_list);
				
				// Add cost function for trajectory profile (speed, jerk, etc.)
				cost_for_state += cost_function_1(traj);
				
				//costs[state_iter] = cost_for_state;

				if (cost_for_state <= min_cost)
				{
					min_cost = cost_for_state;
					best_next_state = state_iter;
					best_next_trajectory = &traj;
				}
				// TODO!
				return best_next_trajectory->getXY(maptool);
			}
		}
	}

	//Trajectory traj = Trajectory(LK, egoPose);
	//return traj.getXY(maptool);
	

	//std::cout << "LCL:" << costs[LCL] << "\tLK:" << costs[LK] << "\tLCR:" << costs[LK] << std::endl;
}






/*
 *	Old code from playing around with splines:
 */

/*vector<vector<double>> StateMachine::generate_trajectory(state proposed_state, pose egoPose, vector<vector<double>> target_vehicles)
{
	// Define anchor points here:
	vector<vector<double>> anchor_vals(2);
	vector<vector<double>> trajectory(2);

	// Define "anchor waypoints" 30, 60, and 90 meters in front of the car:
	anchor_vals[0].push_back( egoPose.pos_x );
	anchor_vals[1].push_back( egoPose.pos_y );

	double d_1, d_2, d_3;

	d_1 = 4*intended_lane+2;
	d_2 = 4*intended_lane+2;
	d_3 = 4*intended_lane+2;

	vector<double> tmp(2);
	tmp = maptool.getXY(egoPose.s+30, d_1);
	anchor_vals[0].push_back( tmp[0] );
	anchor_vals[1].push_back( tmp[1] );
	tmp = maptool.getXY(egoPose.s+60, d_2);
	anchor_vals[0].push_back( tmp[0] );
	anchor_vals[1].push_back( tmp[1] );
	tmp = maptool.getXY(egoPose.s+90, d_3);
	anchor_vals[0].push_back( tmp[0] );
	anchor_vals[1].push_back( tmp[1] );


	// Target speed (in m/s):
	const double target_inc = 0.427;

	bool vehicle_ahead = false;

    for (int i=0; i<target_vehicles.size(); i++)
    {
	    double target_d = target_vehicles[i][6];
	    // Check if target vehicle is on the same lane
	    if ( target_d > (d_3-2) && target_d < (d_3+2) )
	    {
	        double target_s = target_vehicles[i][5];
			double target_vx = target_vehicles[i][3];
	        double target_vy = target_vehicles[i][4];
	        double target_speed = sqrt(target_vx*target_vx+target_vy*target_vy);

	        // Check is target vehicle is on collision course
	        if ( (egoPose.s-target_s)<50 && egoPose.s-target_s>0 && target_speed <= 50*dist_inc)
	        	vehicle_ahead = true;
	    }
	}


	if (vehicle_ahead) {
		dist_inc -= 0.02;
		cout << "Vehicle ahead!" << endl;
	}
	else if (dist_inc <= target_inc)
		dist_inc += 0.04;


	global2vehicle(anchor_vals, egoPose);
	
	//for (int i=0; i<anchor_vals[0].size(); i++)
	//    cout << "anchor_vals: " << anchor_vals[0][i] << "\t" << anchor_vals[1][i] << endl;

	// Calculate splines:
	tk::spline spl;
	spl.set_points(anchor_vals[0], anchor_vals[1]);

	// Generate trajectory:
	double dist = distance(0, 0, 30, spl(30));
	int N = int (dist / dist_inc);

	// In order to eliminate jiggering in longitudinal direction.
	for(int i=1; i-1<150-remaining_points; i++)
	{		      
		trajectory[0].push_back(i*dist_inc);
		trajectory[1].push_back(spl(i*dist_inc));
	}

	vehicle2global(trajectory, egoPose);

	return trajectory;
}*/

