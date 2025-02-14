// ======================================================================
// \title  Main.cpp
// \brief main program for the F' application. Intended for CLI-based systems (Linux, macOS)
//
// ======================================================================
// Used to access topology functions
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/printk.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/usbd.h>
#include <Fw/Logger/Logger.hpp>
#include <LedBlinker/Top/LedBlinkerTopology.hpp>
#include <LedBlinker/Top/LedBlinkerTopologyAc.hpp>
#include <cerrno>
#include "FpConfig.h"
#include "Os/Os.hpp"
#include "Os/Task.hpp"
#include "zephyr/kernel.h"

#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
#define FW_TASK_STACK_SIZE 2048  // Match what Zephyr actually allocates

int blink() {
    return 0;
}

const struct device* serial = DEVICE_DT_GET(DT_NODELABEL(cdc_acm_uart0));

int main() {
    int ret;
    bool led_state = true;
    if (!gpio_is_ready_dt(&led0)) {
        return 0;
    }
    if (!gpio_is_ready_dt(&led1)) {
        return 0;
    }
    gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
    gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);

    gpio_pin_set_dt(&led0, 1);
    gpio_pin_set_dt(&led1, 1);

    /* Initialize USB */
    ret = usb_enable(NULL);
    if (ret < 0 && ret != -EALREADY) {
        return -1;
    }
    gpio_pin_set_dt(&led1, 0);
    gpio_pin_set_dt(&led0, 0);

    /* Wait for USB enumeration */
    while (!device_is_ready(serial)) {
        k_msleep(3000);
        gpio_pin_toggle_dt(&led0);
        gpio_pin_toggle_dt(&led1);
    }

    Os::init();
    Fw::Logger::log("Program Started\n");

    gpio_pin_set_dt(&led1, 1);
    gpio_pin_set_dt(&led0, 0);

    // Object for communicating state to the reference topology
    LedBlinker::TopologyState inputs;
    inputs.dev = serial;
    inputs.uartBaud = 115200;

    // Setup topology
    LedBlinker::setupTopology(inputs);
    Fw::Logger::log("Setup topology\n");

    gpio_pin_set_dt(&led1, 0);
    gpio_pin_set_dt(&led0, 1);

    Fw::Logger::log("Trying to start\n");
    Os::Task::getSingleton().onStart();
    Os::Task::Status status;
    // status = Os::Task::getSingleton().join();
    // Fw::Logger::log("Join attempted %d\n", status);
    // Os::Task::State state = Os::Task::getSingleton().getState();
    // FwSizeType nt = Os::Task::getSingleton().getNumTasks();
    // do {
    // 	gpio_pin_toggle_dt(&led0);
    // 	gpio_pin_toggle_dt(&led1);
    //     k_msleep(500);
    //     Os::Task::State state = Os::Task::getSingleton().getState();
    //     FwSizeType pri = Os::Task::getSingleton().getPriority();
    //     Fw::Logger::log("State: %d, numTasks: %d, pri: %d\n", state, nt, pri);
    // } while(state != Os::Task::State::RUNNING && (nt != 0 && Os::Task::getSingleton().hasRegistry()));

    gpio_pin_set_dt(&led1, 0);
    gpio_pin_set_dt(&led0, 0);

    int cycleCnt = 0;
    while (true) {
        gpio_pin_toggle_dt(&led0);
        gpio_pin_toggle_dt(&led1);
        rateDriver.cycle();
        // if (Os::Task::getSingleton().hasRegistry()) {
        //     Os::Task::getSingleton().invokeRoutine();
        // }
        k_msleep(1);
    }

    return 0;
}
