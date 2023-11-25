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

    int32_t sleep_time;        //Valid while result is wait
    condition_t condition;  //Vail while result is condition

    void* data;
};


struct ThreadCommand
{
    bool roa; // remove or add
    TaskCycle* data;
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
        if(multicore_fifo_rvalid())
        {
            auto rd_command = (ThreadCommand*)multicore_fifo_pop_blocking();
            if(rd_command->roa)
            {
                for(int i=0;i<core1_list->size();i++)
                {
                    if((*core1_list)[i].id == rd_command->data->id)
                        core1_list->erase(core1_list->begin() + i);
                }
            }
            else
            {
                core1_list->push_back(*rd_command->data);
            }

            delete rd_command->data;
            delete rd_command;
        }

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
        while (!multicore_fifo_wready()) sleep_ms(1);

        uint id = 0;
        for(id = 0; id < 65535 ; id++)
        {
            if(!isIdInTasks(id))break;
        }

        auto add_data = new TaskCycle{
                .handler = handler,
                .id = id,
                .is_enable = true,
                .condition = nullptr,
                .data = nullptr
        };

        auto add_command = new ThreadCommand{
                .roa = false,  // Add
                .data = add_data
        };

        multicore_fifo_push_blocking((uint32_t)add_command);

        return id;
    }

    void removeHandler(uint id)
    {
        if(!isIdInTasks(id)) return;
        while (!multicore_fifo_wready()) sleep_ms(1);

        auto add_data = new TaskCycle{
                .id = id      // id to remove
        };

        auto add_command = new ThreadCommand{
                .roa = true,  // Remove
                .data = add_data
        };

        multicore_fifo_push_blocking((uint32_t)add_command);
    }

    void start() const
    {
        if(Defines.core == 1)
        {
            ::core1_list = Tasks;
            multicore_reset_core1();
            multicore_launch_core1(core1_task_f);
        }
    }
};

#endif //PICO_DEMO2_TASK_HPP
