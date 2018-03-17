#ifndef DUCKHUNT_TIMER_HPP
#define DUCKHUNT_TIMER_HPP

class Timer {
private:
    double target;
    double time;

public:
    /// Creates timer.
    /// \param target Target time in ms.
    Timer(double target) {
        this->target = target;
        time = 0.0;
    }

    bool tick(double deltaTime) {
        time += deltaTime;
        if (time > target) {
            time = 0.0;
            return true;
        }
        return false;
    }

    void reset() {
        reset(target);
    }

    void reset(double target) {
        this->target = target;
        time = 0.0;
    }
};

#endif //DUCKHUNT_TIMER_HPP
