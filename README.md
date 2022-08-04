DISCONTINUATION OF PROJECT.

This project will no longer be maintained by Intel.

Intel has ceased development and contributions including, but not limited to, maintenance, bug fixes, new releases, or updates, to this project. 

Intel no longer accepts patches to this project.

If you have an ongoing need to use this project, are interested in independently developing it, or would like to maintain patches for the open source software community, please create your own fork of this project. 
# Filesystem plugin for the parameter-framework

This is a file system plugin for the
[parameter-framework](https://github.com/01org/parameter-framework)
handling file system operations such as `read()` and `write()`.


## Compiling

You need to install the parameter-framework libraries
and headers first (see the parameter-framework's
[README](https://github.com/01org/parameter-framework/blob/master/README.md)).

Generate the Makefiles with `cmake .` and build with `make`.
If you installed the parameter-framework in a custom directory, you should add a
`-DCMAKE_PREFIX_PATH=/path/to/custom/install` argument to `cmake` e.g:
`cmake -DCMAKE_PREFIX_PATH=/home/myself/dev/pfw .` .

If you want to install the plugin to a custom directory, you can add a
`-DCMAKE_INSTALL_PREFIX=/path/to/install/dir` argument to `cmake` e.g:
`cmake -DCMAKE_INSTALL_PREFIX=/home/myself/dev/pfw .` .

Finally, install the libraries with `make install` .


## Usage example

In this example, we are going to control two leds (*ledA* and *ledB*) via sysfs
on a raspberry pi. Let's imagine that the leds are connected on the *pin4* and
*pin7* on the raspberry's GPIO.

To enable sysfs for the pins, we must "export the pins" first:

    echo "4" > /sys/class/gpio/export
    echo "7" > /sys/class/gpio/export

### Configuration file

Here is the complete `rpifs-subsystem.xml` file:
```xml
<?xml version="1.0" encoding="UTF-8"?>
<Subsystem Name="RaspberryFS" Type="FS" Endianness="Little">
    <ComponentLibrary>
        <ComponentType Name="GpioPin">
            <StringParameter Name="direction" Mapping="File:direction" MaxLength="3"
             Description="the direction of the pin: can be out or in" />
            <StringParameter Name="state" Mapping="File:value" MaxLength="1"
             Description="the state of the pin: can be 0 or 1" />
        </ComponentType>
    </ComponentLibrary>
    <InstanceDefinition>
        <Component Name="ledA" Type="GpioPin" Mapping="Directory:/sys/class/gpio/gpio4"/>
        <Component Name="ledB" Type="GpioPin" Mapping="Directory:/sys/class/gpio/gpio7"/>
    </InstanceDefinition>
</Subsystem>
```

When declaring an `InstanceDefinition` the `Directory` mapping must be given to
the component. Here it points towards the directory containing the GPIO pin4
sysfs directory.

When describing a `ComponentType`, the `File:...` mapping must be given to each
parameter. Here we have two parameters. Since we are describing a pin, we must
provide the direction and the state.

### Usage
If we consider that the `pfw` command is a shorthand for `remote-process
localhost 5001`(where 5001 is the port defined at the top-level
parameter-framework configuration file), which can be used to interact with the
parameter-framework.

#### To initialize the leds:

    pfw setTuningMode on
    pfw setParameter /Raspberry/RaspberryFS/ledA/direction out
    pfw setParameter /Raspberry/RaspberryFS/ledB/direction out

#### To turn the led A on:

    pfw setParameter /Raspberry/RaspberryFS/ledA/value 1

#### To turn the led A off:

    pfw setParameter /Raspberry/RaspberryFS/ledA/value 0

