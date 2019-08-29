//
// Created by e on 14/08/18.
//

#ifndef STEAMGIFTSBOT_DURATION_H
#define STEAMGIFTSBOT_DURATION_H

#include <chrono>
#include <iostream>

class Duration{
private:
    typedef std::chrono::high_resolution_clock Time;
    typedef std::chrono::milliseconds ms;
    typedef std::chrono::duration<float> fsec;


    std::string msg;
    Time::time_point t0;
    Time::time_point t1;
    ms d{};
    fsec fs{};
public:
    Duration(){
        t0 = Time::now();
    }

    explicit Duration(const std::string& msg){
		this->msg = msg;
        t0 = Time::now();
    }


    ~Duration(){
        t1 = Time::now();
        fs = t1 - t0;
        d = std::chrono::duration_cast<ms>(fs);
        auto tmp = d.count();
       // if(tmp > 0)
        std::wcout << std::wstring(msg.begin(), msg.end()) << L" ms taken: " << tmp << L'\t' <<  std::endl;
        std::wcout << L'\t' << std::wstring(msg.begin(), msg.end()) << L" ns taken: " << std::chrono::duration_cast<std::chrono::nanoseconds>(fs).count() <<  std::endl;
    }
};

#endif //STEAMGIFTSBOT_DURATION_H
