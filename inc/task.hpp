//
// Created by acite on 23-11-19.
//

#ifndef PICO_DEMO2_TASK_HPP
#define PICO_DEMO2_TASK_HPP

#include <iostream>
#include <vector>

#include "pico/multicore.h"
#include "hardware/timer.hpp" //use for sleep, as a alarm

struct TaskCycle;

typedef bool(*condition_t) (void*);
typedef volatile TaskCycle(*task_t) (TaskCycle Cycle);

enum CycleResult
{
    death,
    alive,
    wait,   //sleep for fixed time, then continue
    condition     //wait for a delegate goto true
};

struct TaskCycle
{
    task_t handler;
    uint id;
    bool is_enable;

    CycleResult result;

    int32_t sleep_time;        //Valid while result is sleep
    condition_t condition;  //Vail while result is wait

    void* data;
};

struct TaskDefines
{
    uint core;
};

static int64_t alarm_set_bool_after_us(alarm_id_t id, void *user_data) {
    *(bool*)user_data = true;
    return 0;
}

static std::vector<TaskCycle>* core1_list = nullptr; // readonly for core1_task_f
static void core1_task_f()
{
    while(1)
    {
        auto load_i = *core1_list;
        for(TaskCycle t : load_i)
        {
            if(!t.is_enable){
                if(!t.condition)
                    continue;
                if(t.condition(t.data))
                {
                    t.condition = nullptr;
                }
                else
                {
                    continue;
                }
            }

            TaskCycle r = t.handler(t);

            if(r.result == alive)
            {
                continue;
            }else if(r.result == death)
            {
                for(int i=0;i<core1_list->size();i++)
                {
                    if((*core1_list)[i].id == r.id)
                        core1_list->erase(core1_list->begin() + i);
                }
                break;
            }else if(r.result == wait)
            {
                for(int i=0;i<core1_list->size();i++)
                {
                    if((*core1_list)[i].id == r.id)
                    {
                        (*core1_list)[i].is_enable = false;

                        AlarmTimer Alarm_Wait(r.sleep_time, alarm_set_bool_after_us, MP_TIMER_IN_US);
                        Alarm_Wait.setUserData(&(*core1_list)[i].is_enable);
                        Alarm_Wait.setEnabled(true);
                    }
                }
                continue;
            }else if(r.result == condition)
            {
                for(int i=0;i<core1_list->size();i++)
                {
                    if((*core1_list)[i].id == r.id)
                    {
                        (*core1_list)[i].is_enable = false;
                        (*core1_list)[i].condition = r.condition;
                    }
                }
                continue;
            }
        }
    }
}

class Task
{
private:
    bool isIdInTasks(uint id)
    {
        for(auto i : *Tasks)
        {
            if(i.id == id)
                return true;
        }

        return false;
    }

    uint getIndex(uint id)
    {
        for(int i=0;i<Tasks->size();i++)
        {
            if((*Tasks)[i].id == id)
                return i;
        }
        return 0xFFFFFFFF;
    }

private:
    TaskDefines Defines { };
    std::vector<TaskCycle>* Tasks;
public:

    explicit Task(TaskDefines Defines = { 1 })
    {
        this->Defines = Defines;
        Tasks = new std::vector<TaskCycle>(0);
    }

    ///
    /// \param handler the handler
    /// \return id
    uint addHandler(task_t handler)
    {
        uint id = 0;
        for(id = 0; id < 65535 ; id++)
        {
            if(!isIdInTasks(id))break;
        }

        Tasks->push_back({
            .handler = handler,
            .id = id,
            .is_enable = true,
            .condition = nullptr,
            .data = nullptr
        });

        return id;
    }

    void removeHandler(uint id)
    {
        if(!isIdInTasks(id)) return;

        Tasks->erase(Tasks->begin() + getIndex(id));
    }

    void start() const
    {
        if(Defines.core == 1)
        {
            ::core1_list = Tasks;
            multicore_launch_core1(core1_task_f);
        }
    }
};

#endif //PICO_DEMO2_TASK_HPP
