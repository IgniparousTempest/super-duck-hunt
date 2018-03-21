#ifndef DUCKHUNT_TIMER_HPP
#define DUCKHUNT_TIMER_HPP

class Timer {
private:
    double target;
    double time;
    bool isEnabled;

public:
    /// Creates timer.
    /// \param target Target time in ms.
    explicit Timer(double target) {
        this->target = target;
        time = 0.0;
        isEnabled = true;
    }

    bool tick(double deltaTime) {
        if (isEnabled) {
            time += deltaTime;
            if (time > target) {
                time = 0.0;
                return true;
            }
        }
        return false;
    }

    void enable() {
        isEnabled = true;
    }

    void disable() {
        isEnabled = false;
    }

    void reset() {
        enable();
        reset(target);
    }

    void reset(double target) {
        this->target = target;
        time = 0.0;
    }
};

#endif //DUCKHUNT_TIMER_HPP
