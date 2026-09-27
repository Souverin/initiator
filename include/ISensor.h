#ifndef ISENSOR_H
#define ISENSOR_H

class ISensor {
public:
    virtual ~ISensor() = default;
    virtual bool init() = 0;
    virtual bool readData() = 0;
    virtual bool isHealthy() const = 0;
    virtual const char* getName() const = 0;
};

#endif // ISENSOR_H