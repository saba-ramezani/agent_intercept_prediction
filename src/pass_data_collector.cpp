//
// Created by sabar on 2/5/2022.
//



/////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "pass_data_collector.h"
#include "strategy.h"
#include "bhv_basic_tackle.h"
#include <rcsc/action/body_intercept.h>
#include <rcsc/common/server_param.h>
#include "ctime"
#include "bhv_block.h"

#include "strategy.h"

#include "bhv_basic_tackle.h"

#include <rcsc/action/basic_actions.h>
#include <rcsc/action/body_go_to_point.h>
#include <rcsc/action/body_intercept.h>
#include <rcsc/action/neck_turn_to_ball_or_scan.h>
#include <rcsc/action/neck_turn_to_low_conf_teammate.h>

#include <rcsc/player/player_agent.h>
#include <rcsc/player/debug_client.h>
#include <rcsc/player/intercept_table.h>

#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>

#include "neck_offensive_intercept_neck.h"

using namespace rcsc;

/*-------------------------------------------------------------------*/

std::vector<data> Pass_Data_Collector::pass_data;
int Pass_Data_Collector::last_kicker_unum;
std::string Pass_Data_Collector::csv_file_name;
bool Pass_Data_Collector::fileMade = false;

void
Pass_Data_Collector::update_data(const WorldModel &wm)
{
    if (wm.self().unum() != 2) {
        dlog.addText(Logger::PASS,
                     __FILE__"Im not player 2");
        return;
    }
    if (wm.gameMode().type() != GameMode::PlayOn) {
        dlog.addText(Logger::PASS,
                     __FILE__"GameMode is not playOn");
        pass_data.clear();
        return;
    }
    if (wm.lastKickerSide() == wm.ourSide()) {
        dlog.addText(Logger::PASS,
                     __FILE__"We kicked the ball last");
        pass_data.clear();
        return;
    }
    //if (wm.interceptTable()->fastestOpponent() == NULL) {

    //}
//    if (wm.interceptTable()->fastestOpponent()->unum() == last_kicker_unum) {
//        dlog.addText(Logger::PASS,
//                     __FILE__"Opponent is dribbling");
//        return;
//    }

    if (!fileMade) {
        set_column_names_of_csv_file();
        fileMade = true;
        dlog.addText(Logger::PASS,
                     __FILE__"The file has been created");
    }
    //Insert inputs
    struct data cycle_data;
    insert_inputs(wm, cycle_data);
    pass_data.push_back(cycle_data);
    dlog.addText(Logger::PASS,
                 __FILE__"The cycle data has been inserted to the vector");
    if (wm.existKickableOpponent()) {
        if (pass_data.size() > 1 || pass_data[0].i_ball_speed > 0) {
            dlog.addText(Logger::PASS,
                         __FILE__"Opponent pass finished");
            //updateLabels
            update_labels(wm);
            dlog.addText(Logger::PASS,
                         __FILE__"The pass labels has been updated");
            //writeToFile
            write_to_file();
            dlog.addText(Logger::PASS,
                         __FILE__"The vector has been added to file");
        }
        //DeleteVector
        pass_data.clear();
        dlog.addText(Logger::PASS,
                     __FILE__"The vector has been cleared");


        //insert_inputs(wm,cycle_data);//???
        //pass_data.push_back(cycle_data);
    }
    if (wm.existKickableOpponent()) {
        last_kicker_unum = wm.getOpponentNearestToBall(5,false)->unum();
    }
}


void
Pass_Data_Collector::insert_inputs(const WorldModel &wm , struct data &cycle_data) {
    cycle_data.cycle = wm.time().cycle();
    Vector2D ball_position = wm.ball().pos();
    cycle_data.i_ball_x = ball_position.x;
    cycle_data.i_ball_y = ball_position.y;
    cycle_data.i_ball_speed = wm.ball().vel().r();
    cycle_data.i_ball_vel_angle = wm.ball().vel().th().degree();
    cycle_data.i_ball_polar_r = ball_position.r();
    cycle_data.i_ball_polar_theta = ball_position.th().degree();
    Vector2D pass_origin ;
    if (pass_data.empty()) //it means that we are at the cycle that the pass begins in
    {
        pass_origin = ball_position;
        cycle_data.i_pass_origin_x = pass_origin.x;
        cycle_data.i_pass_origin_y = pass_origin.y;
    }
    else //it means that we are in the middle of pass
    {
        pass_origin.x = pass_data[0].i_pass_origin_x;
        pass_origin.y = pass_data[0].i_pass_origin_y;
        cycle_data.i_pass_origin_x = pass_origin.x;
        cycle_data.i_pass_origin_y = pass_origin.y;
    }
    cycle_data.i_ball_dist_to_origin = ball_position.dist(pass_origin);
    cycle_data.i_expected_receive_position_x = wm.ball().inertiaPoint(wm.interceptTable()->opponentReachCycle()).x;
    cycle_data.i_expected_receive_position_y = wm.ball().inertiaPoint(wm.interceptTable()->opponentReachCycle()).y;
    const PlayerObject* receiver = wm.interceptTable()->fastestOpponent();
    cycle_data.i_receiver_x = receiver->pos().x;
    cycle_data.i_receiver_y = receiver->pos().y;
    cycle_data.i_receiver_unum = receiver->unum();
    cycle_data.i_receiver_polar_r = receiver->pos().r();
    cycle_data.i_receiver_polar_theta = receiver->pos().th().degree();
    Vector2D receiver_relative_to_ball(receiver->pos().x - ball_position.x , receiver->pos().y - ball_position.y);
    cycle_data.i_receiver_relative_to_ball_x = receiver_relative_to_ball.x;
    cycle_data.i_receiver_relative_to_ball_y = receiver_relative_to_ball.y;
    cycle_data.i_receiver_relative_to_ball_polar_r = receiver_relative_to_ball.r();
    cycle_data.i_receiver_relative_to_ball_polar_theta = receiver_relative_to_ball.th().degree();
    for (int i=1 ; i<12 ; i++)
    {
//        if (wm.theirPlayer(i) == NULL) {
//            cycle_data.i_players_x[i-1] = NULL;
//            cycle_data.i_players_y[i-1] = NULL;
//            continue;
//        }
        Vector2D opponent_pos = wm.theirPlayer(i)->pos();
        cycle_data.i_players_x[i-1] = opponent_pos.x;
        cycle_data.i_players_y[i-1] = opponent_pos.y;
    }
}


void
Pass_Data_Collector::update_labels(const WorldModel &wm) {
    Vector2D receive_pos = wm.ball().pos();
    for (int i=0 ; i<pass_data.size() ; i++) {
        pass_data[i].o_receive_position_x = receive_pos.x;
        pass_data[i].o_receive_position_y = receive_pos.y;
        Vector2D ball_pos_in_cycle_i(pass_data[i].i_ball_x , pass_data[i].i_ball_y);
        pass_data[i].o_pass_distance = ball_pos_in_cycle_i.dist(receive_pos);
    }
}


void
Pass_Data_Collector::set_column_names_of_csv_file() {
    std::time_t result = std::time(nullptr);
    std::string time = std::asctime(std::localtime(&result));
    int randomNum = rand();
    csv_file_name = "pass_log_" + time + "_" + randomNum + ".csv";
    //csv_file_name = "pass_log.csv";
    std::ofstream myFile(csv_file_name);
    //setting column names
    myFile << "cycle" << ",";
    myFile << "i_ball_x" << "," << "i_ball_y" << ",";
    myFile << "i_ball_speed" << "," << "i_ball_vel_angle" << ",";
    myFile << "i_ball_polar_r" << "," << "i_ball_polar_theta" << ",";
    myFile << "i_pass_origin_x" << "," << "i_pass_origin_y" << ",";
    myFile << "i_ball_dist_to_origin" << ",";
    myFile << "i_expected_receive_position_x" << "," << "i_expected_receive_position_y" << ",";
    myFile << "i_receiver_x" << "," << "i_receiver_y" << ",";
    myFile << "i_receiver_unum" << ",";
    myFile << "i_receiver_polar_r" << "," << "i_receiver_polar_theta" << ",";
    myFile << "i_receiver_relative_to_ball_x" << "," << "i_receiver_relative_to_ball_y" << ",";
    myFile << "i_receiver_relative_to_ball_polar_r" << "," << "i_receiver_relative_to_ball_polar_theta" << ",";
    for (int i=1 ; i<12 ; i++) {
        myFile << "i_player_" << i << "_x" << "," << "i_player_" << i << "_y" << "," ;
    }
    myFile << "o_receive_position_x" << "," << "o_receive_position_y" << ",";
    myFile << "o_pass_distance" << "\n" ;
    myFile.close();
}


void
Pass_Data_Collector::write_to_file() {
    std::ofstream myFile;
    myFile.open(csv_file_name,std::ios_base::app);
    for (int i=0 ; i<pass_data.size() ; i++) {
        struct data dataToWrite = pass_data[i];
        //std::cout<<"printing\n";
        myFile << dataToWrite.cycle << ",";
        myFile << dataToWrite.i_ball_x << "," << dataToWrite.i_ball_y << ",";
        myFile << dataToWrite.i_ball_speed << "," << dataToWrite.i_ball_vel_angle << ",";
        myFile << dataToWrite.i_ball_polar_r << "," << dataToWrite.i_ball_polar_theta << ",";
        myFile << dataToWrite.i_pass_origin_x << "," << dataToWrite.i_pass_origin_y << ",";
        myFile << dataToWrite.i_ball_dist_to_origin << ",";
        myFile << dataToWrite.i_expected_receive_position_x << "," << dataToWrite.i_expected_receive_position_y << ",";
        myFile << dataToWrite.i_receiver_x << "," << dataToWrite.i_receiver_y << ",";
        myFile << dataToWrite.i_receiver_unum << ",";
        myFile << dataToWrite.i_receiver_polar_r << "," << dataToWrite.i_receiver_polar_theta << ",";
        myFile << dataToWrite.i_receiver_relative_to_ball_x << "," << dataToWrite.i_receiver_relative_to_ball_y << ",";
        myFile << dataToWrite.i_receiver_relative_to_ball_polar_r << "," << dataToWrite.i_receiver_relative_to_ball_polar_theta << ",";
        for (int j=1 ; j<12 ; j++) {
            myFile << dataToWrite.i_players_x[j-1] << "," << dataToWrite.i_players_y[j-1] << "," ;
        }
        myFile << dataToWrite.o_receive_position_x << "," << dataToWrite.o_receive_position_y << ",";
        myFile << dataToWrite.o_pass_distance << "\n" ;
    }
    myFile.close();
}


