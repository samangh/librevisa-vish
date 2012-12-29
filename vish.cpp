#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <visa.h>

#include <iostream>

#include <cstring>

int main(int argc, char **argv)
{
        if(argc != 2)
        {
                std::cerr << "Usage: " << argv[0] << " VISA-RESOURCE" << std::endl;
                return 1;
        }

        ViSession rmgr;

        if(viOpenDefaultRM(&rmgr) != VI_SUCCESS)
        {
                std::cerr << "E: Cannot open default resource manager" << std::endl;
                return 1;
        }

        ViSession vi;

        if(viOpen(rmgr, argv[1], VI_NO_LOCK, 0, &vi) != VI_SUCCESS)
        {
                std::cerr << "E: Cannot open resource " << argv[1] << std::endl;
                return 1;
        }

        bool rigol_stb_workaround = false;

        {
                char buffer[256] = "*IDN?";
                ViUInt32 count;
                if(viWrite(vi, reinterpret_cast<ViPBuf>(buffer), 5, &count) != VI_SUCCESS ||
                        viRead(vi, reinterpret_cast<ViPBuf>(buffer), sizeof buffer, &count) != VI_SUCCESS)
                {
                        std::cerr << "E: Cannot query device type" << std::endl;
                        return 1;
                }
                if(count >= 5 && !strncmp(buffer, "Rigol", 5))
                        rigol_stb_workaround = true;
        }

        for(std::string line; getline(std::cin, line) && line.size();)
        {
                ViUInt32 count;
                if(viWrite(vi, reinterpret_cast<ViPBuf>(const_cast<char *>(line.data())), line.size(), &count) != VI_SUCCESS)
                        std::cerr << "E: Cannot write to resource" << std::endl;

                for(;;)
                {
                        ViUInt16 status;
                        if(!rigol_stb_workaround && viReadSTB(vi, &status) != VI_SUCCESS)
                                std::cerr << "E: Cannot read status" << std::endl;

                        if(rigol_stb_workaround || status & 16)
                        {
                                ViChar buffer[256000];
                                if(viRead(vi, reinterpret_cast<ViPBuf>(buffer), sizeof buffer, &count) != VI_SUCCESS)
                                        break;
                                std::cout << std::string(buffer, count);
                        }
                        else
                                break;
                }
        }

        viClose(vi);
        viClose(rmgr);

        return 0;
}
