#include <cstdarg>
#include <cstdio>

#include "logs.hpp"

//--------------------------------------------------------------

static const char BACKGROUND_IMG[]  = "url(https://catherineasquithgallery.com/uploads/posts/2021-02/1614279388_81-p-chernii-fon-dlya-stima-96.jpg)";

const char START_LOGFILE[] =    "<style>\n"
                                    "\tsummary {\n"
                                        "\t\ttext-align: center;\n"
                                        "\t\tcolor: rgb(250, 125, 0);\n"
                                        "\t\tborder: 1px solid rgb(250, 125, 0);\n"
                                        "\t\tborder-radius: 15px;font-weight: bold;\n"
                                        "\t\tmargin: .5em -.5em 0;\n"
                                        "\t\tpadding: .5em;\n"
                                    "\t}\n"

                                    "\tbody {\n"
                                        "\t\tbackground: %s;\n"
                                        "\t\tbackground-size: cover;\n"
                                    "\t}\n"

                                    "\tp {\n"
                                        "\t\tfont-size: 25px;\n"
                                        "\t\tcolor: white;\n"
                                    "\t}\n"
                                "</style>\n\n";

static const char LOG_FILENAME[] = "./Log/log.html";
static FILE *Logfile = nullptr;

//--------------------------------------------------------------

#ifdef LOGS
    void initLog()
    {
        Logfile = fopen(LOG_FILENAME, "w");
        if (!Logfile)
        {
            printf("Error opening logfile");
            return;
        }

        setvbuf(Logfile, nullptr, _IONBF, 0);

        fprintf(Logfile, START_LOGFILE, BACKGROUND_IMG);
    }

    int log(const char *format, ...)
    {
        if (Logfile == nullptr) {return -1;}

        va_list ptr = {};
        va_start(ptr, format);

        int result = vfprintf(Logfile, format, ptr);

        va_end(ptr);
        return result;
    }

    void closeLog()
{
    fclose(Logfile);
}
#else
    void initLog(){}
    int log(const char *format, ...){return 0;}
    void closeLog(){}
#endif //LOGS