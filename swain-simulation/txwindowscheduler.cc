#include "txwindowscheduler.h"

GlobalVariables tx_gv;


/**
 * Identifies the expected arrival time of the specified gateway to each one of the end-devices. 
 * Assigns these times to the Node structure itself.
 * Assumes that the HAPS takes circular motion across the field for ease of calculation.
 * Circle period is used for creating periodic behaviour
 * Do take care that some devices will be in range in multiple intervals due to circle movement (max 2).
 * TODO: Dont forget the 10 daily message limit
 */
void TXWindowScheduler::IdentifyRangeTimers (NodeContainer gateways, NodeContainer& enddevices, double simDuration)
{
    // Loop through time
    int n_gateways = gateways.GetN ();
    bool haps_assigned [n_gateways];

    // logs
    int total_assignment_done = 0;
    int total_range_counter = 0;
    int total_haps_availability_counter = 0;
    int total_assignment_expectation = 0;

    int sim_timer;
    for (sim_timer=0; sim_timer < simDuration; sim_timer++) 
    {
        // bool time_assigned = false;
        // Set all HAPS assigned to false
        for (bool &bb: haps_assigned) {
            bb = false;
        }

        int assignment_done = 0;
        // For each end-device
        for (NodeContainer::Iterator i = enddevices.Begin (); i != enddevices.End (); ++i)
        {
            // Get the end-device
            Ptr<Node> n = (*i);
            Ptr<HAPNode> enddevice = dynamic_cast<HAPNode*>(&*n); 

            // Check if there exists any HAPS which is in range for the given end-device
            // bool HAPSInRange = CheckHAPSInRange (gateways, enddevice, sim_timer);
            list<int> HAPSInRange = CheckHAPSInRange (gateways, enddevice, sim_timer);

            // Check if it is in range
            // If the entering timer is not set, set it. Otherwise do not update anything until 
            // HAPS is out of range. When HAPS is out of range, set the timer as current time - 1
            if (HAPSInRange.size () > 0) {
                total_range_counter++;

                if (enddevice->txWindowList.size () == 0 || enddevice->txWindowList.back ().TxEndTime != -1) {
                    TxInterval newInterval;
                    newInterval.TxStartTime = sim_timer;
                    enddevice->txWindowList.push_back (newInterval);
                }

                // Check if any of the found in-range haps has not been assigned
                // in this timeslot yet, 
                // TODO: each HAPS should be able to take on 6 transmissions from different data-rates at once.
                int available_haps = -1;
                for (int &bb: HAPSInRange) {
                    if (haps_assigned [bb] == false) {
                        available_haps = bb;
                        if (sim_timer % GlobalVariables::txDutyDuration == 0) {
                            total_haps_availability_counter++;
                        }
                        break; // should only set one haps to be true!
                    }
                }

                // Check if the next timeslot has come
                // Timeslot assigment...
                if (sim_timer % GlobalVariables::txDutyDuration == 0 && available_haps != -1) {
                // if (sim_timer % GlobalVariables::txDutyDuration == 0 && time_assigned == false) {
                    total_assignment_expectation++;

                    // TODO: creating new transmission point object
                    TxPoint* txPoint = new TxPoint;
                    txPoint->TxTime = sim_timer;
                    txPoint->used = false;

                    // no element exists yet assign directly
                    if (enddevice->txPointList.size () == 0) {
                    // if (enddevice->txPointListt.size () == 0) {
                        enddevice->txPointListt.push_back (sim_timer);
                        // time_assigned = true;
                        assignment_done++;
                        total_assignment_done++;
                        haps_assigned [available_haps] = true; // this haps should not be used in this second anymore
                        
                        enddevice->txPointList.push_back (*txPoint);
                    }
                    else {
                        // check the last point assigned to that end-device
                        // int lastAssignedPoint = enddevice->txPointListt.back ();
                        TxPoint lastAssignedPoint_new = enddevice->txPointList.back ();
                        // if an entire duty cycle waiting is done we can assign it
                        // else check the next end-device
                        if (lastAssignedPoint_new.TxTime + GlobalVariables::txDutyCycleBackoff <= sim_timer) {
                        // if (lastAssignedPoint + GlobalVariables::txDutyCycleBackoff <= sim_timer) {
                            enddevice->txPointListt.push_back (sim_timer);
                            // time_assigned = true;
                            assignment_done++;
                            total_assignment_done++;
                            haps_assigned [available_haps] = true; // this haps should not be used in this second anymore
                            
                            enddevice->txPointList.push_back (*txPoint);
                        }
                    }
                }
            }
            else {
                if (enddevice->txWindowList.size () > 0 && enddevice->txWindowList.back ().TxEndTime == -1) {
                    enddevice->txWindowList.back ().TxEndTime = sim_timer;
                }
            }
            // Last time point, we need to set the ones who have not finished yet to max time
            if (sim_timer == simDuration - 1) {
                if (enddevice->txWindowList.size () > 0 && enddevice->txWindowList.back ().TxEndTime == - 1) {
                    enddevice->txWindowList.back ().TxEndTime = sim_timer;
                }
            }
        };
    };

    // Set the initial expected transmission times
    for (NodeContainer::Iterator i = enddevices.Begin (); i != enddevices.End (); ++i) {
        // Get the end-device
        Ptr<Node> n = (*i);
        Ptr<HAPNode> enddevice = dynamic_cast<HAPNode*>(&*n); 

        enddevice->last_transmission_expected_start_time = enddevice->txPointList.front ().TxTime;
    }

    NS_LOG_UNCOND ("Number of total assignments done: " << total_assignment_done);
    NS_LOG_UNCOND ("Number of times an ED was in range: " << total_range_counter);
    NS_LOG_UNCOND ("Number of times haps were available: " << total_haps_availability_counter);
    NS_LOG_UNCOND ("Number of expected assignments without duty-cycle consideration: " << total_assignment_expectation);
};

void TXWindowScheduler::PrintEntireSchedule (NodeContainer& enddevices) {
    list<double> allTimers;
    for (NodeContainer::Iterator i = enddevices.Begin (); i != enddevices.End (); ++i)
    {
        Ptr<Node> n = (*i);
        Ptr<HAPNode> enddevice = dynamic_cast<HAPNode*>(&*n);

        for(const auto& timer : enddevice->txPointListt) {
            allTimers.push_back (timer);
            // NS_LOG_UNCOND (enddevice->GetId () << " " << timer);
        }        
    }    
    allTimers.sort ();
    for(const auto& timer : allTimers) {
        NS_LOG_UNCOND (timer);
    }
}

/**
 * Difference between awakening and scheduling is that device needs to stay awake while
 * it is withing the HAPS border.
 */
int TXWindowScheduler::GetEDNextAwakeningTime (Ptr<HAPNode> node, double currentTime)
{
    int awakeningTime = -1;
    for(const auto& window : node->txWindowList) {
        if (window.TxStartTime <= currentTime && currentTime <= window.TxEndTime)
            awakeningTime = currentTime - window.TxStartTime;
        else if (window.TxStartTime > currentTime)
            awakeningTime = window.TxStartTime;
        // A match is found
        if (awakeningTime != -1)
            break;
    }
    return awakeningTime - currentTime;
}

string TXWindowScheduler::PrintEDWindows (Ptr<HAPNode> node)
{
    NS_LOG_UNCOND ("Window list size: " << node->txWindowList.size ());
    for(TxInterval window : node->txWindowList) {
        NS_LOG_UNCOND (node->GetId () << " timers: " << window.TxStartTime <<  " " << window.TxEndTime);
    }

    return "";
}

TxPoint TXWindowScheduler::GetNextTimerForED (Ptr<HAPNode> node, double currentTime) {
    TxPoint* crr_tx_point = new TxPoint;
    crr_tx_point->TxTime = -1;

    bool nextTimeExists = false;
    for(const TxPoint& tx_point : node->txPointList) {
        NS_ASSERT (node->last_transmission_expected_start_time >= 0);
        // we need to check both current time, as well as the last time a transmission series ended
        // if (floor (currentTime) <= tx_point.TxTime && 
        //     ((ceil (node->last_transmission_end_time) + GlobalVariables::txDutyCycleBackoff) <= tx_point.TxTime 
        //      || node->last_transmission_end_time <= 0)) {
        // if (floor (currentTime) <= tx_point.TxTime) {
            
        // Each time this method is called it will return a future time, so be careful on when it is called!
        // TODO: due to the nature of simulation as well as protocol delays can occur and precise time slotting 
        // TODO: becomes a hassle, on top of that the environment is mobile
        if (floor (currentTime) <= tx_point.TxTime && 
            (node->last_transmission_start_time + GlobalVariables::txDutyCycleBackoff <= tx_point.TxTime + 10)
        ) {            
            node->last_transmission_expected_start_time = tx_point.TxTime;
            crr_tx_point->TxTime = tx_point.TxTime;
            nextTimeExists = true;
            break;
        }
    }   
    if (!nextTimeExists) {
        return *crr_tx_point;
    }
    // scheduling is done after (nextTimer - current) seconds
    crr_tx_point->TxTime = crr_tx_point->TxTime - floor (currentTime);
    return *crr_tx_point; 
}

list<int> TXWindowScheduler::CheckHAPSInRange (NodeContainer gateways, Ptr<HAPNode> enddevice, double current_time) {
    list<int> haps_in_range;
    int haps_ctr = 0;
    // Check the range between each gateway and end-device
    for (NodeContainer::Iterator i = gateways.Begin (); i != gateways.End (); ++i)
    {
        // Get the gateway
        Ptr<Node> n = (*i);
        Ptr<HAPNode> gateway = dynamic_cast<HAPNode*>(&*n);

        // Check the range if the current timer is larger than delay so that we can understand if it started travelling
        if (current_time >= gateway->haps_delay) {
            double currentDistance = SensorTopologyManager::CalculateDistanceBetweenNodes (gateway, enddevice);
            if (currentDistance <= GlobalVariables::HAPSRange) {
                haps_in_range.push_back (haps_ctr);
                // return true;
            }
        }
        haps_ctr++;
    }; 
    return haps_in_range;
    // No HAPS in range
    // return false;
};