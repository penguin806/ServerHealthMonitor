#ifndef CONFIGCONTAINER_H
#define CONFIGCONTAINER_H

class ConfigContainer
{
public:
    enum LOGMODE{
        LOG_MODE_LOCAL,
        LOG_MODE_REMOTE
    };
    ConfigContainer();
    void loadConfig();
    void printConfig();

    LOGMODE getLogMode() const;

private:
    LOGMODE logMode;
};

#endif // CONFIGCONTAINER_H
