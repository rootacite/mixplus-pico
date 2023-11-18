//
// Created by acite on 23-11-19.
//

#ifndef PICO_DEMO2_TASK_HPP
#define PICO_DEMO2_TASK_HPP

#include <iostream>
#include <vector>

#include "pico/multicore.h"
#include "timer.hpp" //use for sleep, as a alarm

struct TaskCycle;

typedef bool(*condition_t) (void*);
typedef TaskCycle(*task_t) (TaskCycle Cycle);

enum CycleResult
{
    death,
    alive,
    sleep,   //sleep for fixed time, then continue
    wait     //wait for a delegate goto true
};

struct TaskCycle
{
    task_t handler;
    uint id;

    CycleResult result;

    uint sleep_time;        //Valid while result is sleep
    condition_t condition;  //Vail while result is wait

    void* data;
};


struct TaskDefines
{
    uint core;
};

static std::vector<TaskCycle>* core1_list = nullptr; // readonly for core1_task_f
static void core1_task_f()
{
    while(1)
    {
        for(TaskCycle t : *core1_list)
        {
            TaskCycle r = t.handler(t);

            if(r.result == alive)
            {
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
        Tasks = new std::vector<TaskCycle>;
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
