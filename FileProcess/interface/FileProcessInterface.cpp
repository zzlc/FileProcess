#include "interface/FileProcessInterface.h"
#include "interface/BaseFileProcess.h"

FileProcessInterface* FileProcessInterface::ObtainSingleInterface()
{
    return IBaseFileProcess::ObtainSingleInterface();
}

int FileProcessInterface::ReleaseInterface(FileProcessInterface* interface_ptr)
{
    if (interface_ptr) {
        delete interface_ptr;
        return 0;
    } else {
        return -1;
    }
}
