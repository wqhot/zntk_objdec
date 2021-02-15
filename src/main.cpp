/**
* Copyright 2020 Huawei Technologies Co., Ltd
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at

* http://www.apache.org/licenses/LICENSE-2.0

* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.

* File main.cpp
* Description: tensorflow yolov4 demo
*/
#include <iostream>
#include <fstream>
#include <map>
#include <cstdint>
#include <string>
#include <memory>
#include <unistd.h>

#include "detect_thread.h"
#include "recv_thread.h"


thread_queue::ThreadQueue<basic_item> global_queue;

int main(int argc, const char *argv[]) {
    RecvThread recv_thread(10001);
    DetectThread detect_thread(recv_thread.queue_id, "192.168.50.218", "10003");
    recv_thread.detect_thread = &detect_thread;
    while (1)
    {
        sleep(1);
    }

    // Return success
    return 0;
}
