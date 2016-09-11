/*
 * 2016 Copyright (c) Seeed Technology Inc.  All right reserved.
 * Author:Baozhu Zuo <zuobaozhu@gmail.com>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
*/


#include <iostream>

#include "application.h"
#include "exception.h"

using namespace std;
/*
 *
 */
int main(int argc, char** argv) {

    try {
        dns::Application* application = new dns::Application();
        application->parse_arguments(argc, argv);
        application->run();
    }
    catch (dns::Exception& e) {
        cout << e.what() << endl;
    }
    catch (exception& e) {
        cout << e.what() << endl;
    }

    return 0;
}
