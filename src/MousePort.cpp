#include "pch.hpp"

#include <fcntl.h>
#include <linux/input.h>

#include "utils.hpp"
#include "MousePort.hpp"
#include "MidiEvent.hpp"





MousePort::MousePort() {

    absolute_x = 0;
    absolute_y = 0;

    if ((fd = open(dev, O_RDONLY)) == -1) {
        LOG(LogLvl::ERROR) << "Mouse device file error: " << dev;
        exit(EXIT_FAILURE);
    }
}

void MousePort::get_input_event(MidiEvent& ev) {
    struct input_event ie;
    char x, y;


    while (read(fd, &ie, sizeof(struct input_event)))
    {
        unsigned char* ptr = (unsigned char*)&ie;
        unsigned char bLeft = ptr[0] & 0x1;

        x = (char)ptr[1];
        y = (char)ptr[2];
        printf("rx: %d  ry=%d\n", x, y);

        absolute_x += x;
        absolute_y -= y;

        printf("Absolute coords from TOP_LEFT= %i %i; %i\n", absolute_x, absolute_y, bLeft);
        //
        // comment to disable the display of raw event structure datas
        //
        for (size_t i = 0; i < sizeof(ie); i++) {
            printf("%02X ", *ptr++);
        }
        printf("\n");
    }

}