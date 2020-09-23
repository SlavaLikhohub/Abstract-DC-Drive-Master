#################
Abstract DC Drive
#################

Description
***********

This repository an example of controling DC drive `Abstract-DC-Drive <https://github.com/SlavaLikhohub/Abstract-DC-Drive>`_.
It is based on `Abstract-STM32Fx <https://github.com/SlavaLikhohub/Abstract-STM32Fx>`_ library.

Tested using the **STM32F103 Bluepill**. 
If you are using something else consider reviewing pinouts settings.

Requirements
************

List of requirment provided here `Abstract-STM32Fx <https://github.com/SlavaLikhohub/Abstract-STM32Fx>`_.

How to start
************

#. Make sure you have completed the installation steps described in Requirements_ first.
#. Recursively clone the repository:

   .. code-block:: shell-session
      
      git clone --recursive https://github.com/SlavaLikhohub/Abstract-DC-Drive
      
   or clone first and then initialize all submodules
   
   .. code-block:: shell-session
      
      git clone https://github.com/SlavaLikhohub/Abstract-DC-Drive
      git submodule update --init --recursive
      
#. Check pinouts at beginings of **src/*.c** files.
#. Connect your STM32 to computer.
#. Build and flash the program:
   
.. code-block:: shell-session
      
   make DEVICE=<device> TARGET=<example> PROFILE=release LOG={0,1} tidy all

where ``<device>`` is ``stm32f407vgt6`` or ``stm32f103c8t6``, ``<example>`` is a file from ``src dir`` without extantion (i.e. blinky_stm32f4)


