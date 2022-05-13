//
// Created by sabar on 3/23/2022.
//

#ifndef AGENT2D_3_1_1_PASS_DATA_COLLECTOR_H
#define AGENT2D_3_1_1_PASS_DATA_COLLECTOR_H



#include "action_generator.h"
#include "field_evaluator.h"
#include "communication.h"

#include <rcsc/player/player_agent.h>
#include <vector>

struct data{
    //i for input, o for output
    //ball
    int cycle;
    double i_ball_x, i_ball_y;
    double i_ball_speed;
    double i_ball_vel_angle;
    double i_ball_polar_r, i_ball_polar_theta;
    double i_pass_origin_x, i_pass_origin_y;
    double i_ball_dist_to_origin;
    double i_expected_receive_position_x;
    double i_expected_receive_position_y;
    //receiver
    double i_receiver_x, i_receiver_y;
    int i_receiver_unum;
    double i_receiver_polar_r, i_receiver_polar_theta;
    //receiver position relative to ball
    double i_receiver_relative_to_ball_x, i_receiver_relative_to_ball_y;
    double i_receiver_relative_to_ball_polar_r, i_receiver_relative_to_ball_polar_theta;
    //players
    double i_players_x[11];
    double i_players_y[11];
    //output
    double o_receive_position_x, o_receive_position_y;
    double o_pass_distance;
};

using namespace rcsc;

class Pass_Data_Collector {
public :

    static std::vector<data> pass_data;
    static int last_kicker_unum;
    static std::string csv_file_name;
    static bool fileMade ;

    Pass_Data_Collector()
    {}

    void update_data(const WorldModel &wm);
    void insert_inputs(const WorldModel &wm , struct data &cycle_data);
    void update_labels(const WorldModel &wm);
    void set_column_names_of_csv_file();
    void write_to_file();
private :
};

#endif