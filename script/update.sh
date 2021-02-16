#!/bin/bash

cd ../build
make -j6
cd ../script

scp ../build/out/main HwHiAiUser@192.168.50.137:/home/HwHiAiUser/zntk/zntk_objdec

ssh HwHiAiUser@192.168.50.137 "sudo cp -af /home/HwHiAiUser/zntk/zntk_objdec /opt/zntk/out/zntk_objdec"

