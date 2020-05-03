//
// Created by Chengke Wong on 2020/5/2.
//

#ifndef ALARM_H
#define ALARM_H

#include <functional>

#include "util/time.h"

class AlarmManager;

class Alarm {

public: 

    // |now| is the time when the alarm was fired.
    using Delegte = std::function<void(Instant now)>;

    Alarm(Delegte delegate);

    void set(Instant new_deadline);

    void update(Instant new_deadline,
        Duration granularity = Duration::from_milliseconds(1));

    void cancel();

    bool is_set();

    void fire();

    Alarm (Alarm&&) = default;
    Alarm& operator= (Alarm&&) = default;

    // disallow copy and assignment
    Alarm (const Alarm&) = delete;
    Alarm& operator= (const Alarm&) = delete;

private:

    Delegte delegate_;

};

class AlarmManager {

public: 

    using AlarmMap = std::multimap<Instant, Alarm*>;
    using AlarmToken = AlarmMap::const_iterator;

    AlarmToken register_alarm(Alarm *alarm);

    void unregister_alarm(AlarmToken token);

    template<typename... Args>
    unique_ptr<Alarm> new_alarm(Args&&... args);
        // : std::runtime_error(std::forward<Args>(args)...)

    void fire(Instant now);

    Duration time_to_next_fire();

    AlarmManager (AlarmManager&&) = default;
    AlarmManager& operator= (AlarmManager&&) = default;

    // disallow copy and assignment
    AlarmManager (const AlarmManager&) = delete;
    AlarmManager& operator= (const AlarmManager&) = delete;

private:

    AlarmMap alarms_;

};


#endif
