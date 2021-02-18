#!/bin/bash

cd ../build
cmake -D CMAKE_BUILD_TYPE=RELEASE
make -j6
cd ../script
ssh HwHiAiUser@192.168.50.137 "sudo mkdir -p /opt/zntk/out && sudo mkdir -p /opt/zntk/model"
scp ../build/out/main HwHiAiUser@192.168.50.137:/home/HwHiAiUser/zntk/zntk_objdec
scp ../model/yolov4.om HwHiAiUser@192.168.50.137:/home/HwHiAiUser/zntk/yolov4.om
scp ./zntk_mkdir.service HwHiAiUser@192.168.50.137:/home/HwHiAiUser/zntk/zntk_mkdir.service
scp ./zntk_objdec.service HwHiAiUser@192.168.50.137:/home/HwHiAiUser/zntk/zntk_objdec.service
scp ./run.sh HwHiAiUser@192.168.50.137:/home/HwHiAiUser/zntk/run.sh
ssh HwHiAiUser@192.168.50.137 "sudo cp -af /home/HwHiAiUser/zntk/zntk_objdec /opt/zntk/out/zntk_objdec"
ssh HwHiAiUser@192.168.50.137 "sudo cp -af /home/HwHiAiUser/zntk/yolov4.om /opt/zntk/model/yolov4.om"
ssh HwHiAiUser@192.168.50.137 "sudo cp -af /home/HwHiAiUser/zntk/run.sh /opt/zntk/run.sh"
ssh HwHiAiUser@192.168.50.137 "sudo cp -af /home/HwHiAiUser/zntk/zntk_mkdir.service /opt/zntk/zntk_mkdir.service"
ssh HwHiAiUser@192.168.50.137 "sudo cp -af /home/HwHiAiUser/zntk/zntk_objdec.service /opt/zntk/zntk_objdec.service"

ssh HwHiAiUser@192.168.50.137 "sudo cp -af /opt/zntk/zntk_mkdir.service /etc/systemd/system/zntk_mkdir.service && sudo systemctl enable zntk_mkdir.service"
ssh HwHiAiUser@192.168.50.137 "sudo cp -af /opt/zntk/zntk_objdec.service /etc/systemd/system/zntk_objdec.service && sudo systemctl enable zntk_objdec.service"


# ssh HwHiAiUser@192.168.50.137 "mkdir /opt/zntk"
# scp ../out/main HwHiAiUser@192.168.50.137:/home/HwHiAiUser/zntk/zntk_objdec
# scp ../out/acl.json HwHiAiUser@192.168.50.137:/home/HwHiAiUser/zntk/acl.json
# scp ../model/yolov3.om HwHiAiUser@192.168.50.137:/home/HwHiAiUser/zntk/yolov3.om
# scp ./run.sh HwHiAiUser@192.168.50.137:/home/HwHiAiUser/zntk/run.sh
# scp ./zntk_mkdir.service HwHiAiUser@192.168.50.137:/home/HwHiAiUser/zntk/zntk_mkdir.service
# scp ./zntk_objdec.service HwHiAiUser@192.168.50.137:/home/HwHiAiUser/zntk/zntk_objdec.service

# ssh HwHiAiUser@192.168.50.137 "sudo cp -af /home/HwHiAiUser/zntk/zntk_objdec /opt/zntk/zntk_objdec"
# ssh HwHiAiUser@192.168.50.137 "sudo cp -af /home/HwHiAiUser/zntk/acl.json /opt/zntk/acl.json"
# ssh HwHiAiUser@192.168.50.137 "sudo cp -af /home/HwHiAiUser/zntk/yolov3.om /opt/zntk/yolov3.om"
# ssh HwHiAiUser@192.168.50.137 "sudo cp -af /home/HwHiAiUser/zntk/run.sh /opt/zntk/run.sh"
# ssh HwHiAiUser@192.168.50.137 "sudo cp -af /home/HwHiAiUser/zntk/zntk_mkdir.service /opt/zntk/zntk_mkdir.service"
# ssh HwHiAiUser@192.168.50.137 "sudo cp -af /home/HwHiAiUser/zntk/zntk_objdec.service /opt/zntk/zntk_objdec.service"


# ssh HwHiAiUser@192.168.50.137 "sudo cp -af /opt/zntk/zntk_mkdir.service /etc/systemd/system/zntk_mkdir.service && sudo systemctl enable zntk_mkdir.service"
# ssh HwHiAiUser@192.168.50.137 "sudo cp -af /opt/zntk/zntk_objdec.service /etc/systemd/system/zntk_objdec.service && sudo systemctl enable zntk_objdec.service"